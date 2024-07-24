#include "vulkan_backend.hpp"

#define GLM_FORCE_RADIANS

#include "engine.hpp"
#include "core/defines.h"
#include "core/logger.h"
#include "vk_types.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include <renderer/core/camera.hpp>
#include <renderer/core/render_types.hpp>
#include "vk_forward_render_path.hpp"
#include "vk_deferred_render_path.hpp"

#include <VulkanBootstrap/VkBootstrap.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>
#include <array>

CVulkanBackend::CVulkanBackend() :
	m_bIsInitialized(false),
	m_pVulkanDevice(nullptr),
	m_pVulkanSwapchain(nullptr),
	m_pCurrentRenderPath(nullptr),
	m_CurrentFrame(0),
	m_bWasWindowResized(false)
{
}

bool CVulkanBackend::Initialize()
{
	m_pVulkanDevice = new CVulkanDevice();
	m_pVulkanDevice->InitVulkanDevice();

	m_pVulkanSwapchain = new CVulkanSwapchain(m_pVulkanDevice);
	m_pVulkanSwapchain->InitVulkanSwapchain();

	InitCommandPools();

	InitDescriptorSetLayouts();

	InitDescriptorSetPool();

	InitSyncStructures();

	InitTextureSamplers();

	// m_pForwardRenderPath = new CVulkanForwardRenderPath(this, m_pVulkanDevice, m_pVulkanSwapchain);
	// m_pForwardRenderPath->CreateResources();

	vkutils::LoadImageFromFile(m_pVulkanDevice, "../Resources/Images/viking_room.png", m_Image);
	
	// TODO: Placeholder for testing purposes. TO BE REMOVED.
	VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(m_pVulkanDevice->m_Device, &ViewInfo, nullptr, &m_ImageView));

	InitDescriptorSets();

	m_pCurrentRenderPath = CreateRenderPath();

    m_bIsInitialized = true;

    return true;
}

void CVulkanBackend::Render(const CCamera* const aCamera)
{
	assert(m_bIsInitialized);

	// if (m_pForwardRenderPath)
	// {
	// 	m_pForwardRenderPath->Render(aCamera);
	// }

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->UpdateBuffers();
		m_pCurrentRenderPath->Render(aCamera);
	}
}

bool CVulkanBackend::Shutdown()
{
	m_pCurrentRenderPath->DestroyResources();
	delete m_pCurrentRenderPath;

	if (m_bIsInitialized)
	{
		SGSINFO("Shutting down Vulkan");

		vkDestroyCommandPool(m_pVulkanDevice->m_Device, m_CommandPool, nullptr);
	}

	delete m_pVulkanSwapchain;
	delete m_pVulkanDevice;

    return false;
}

void CVulkanBackend::HandleWindowResize()
{
	m_bWasWindowResized = true;
}

void CVulkanBackend::CreateRenderObjectsData(const std::vector<sRenderObject*>& aRenderObjects)
{
	for (const auto& RenderObject : aRenderObjects)
	{
		sRenderObjectData RenderObjectData;
		RenderObjectData.ModelMatrix = RenderObject->ModelMatrix;
		const sMeshData* MeshData = sMeshData::GetMeshData(RenderObject->pRenderObjectInfo->MeshPath);
		if (MeshData)
		{
			// If there is already a Vulkan representation of that mesh, use it instead of creating a copy of the same data.
			// If not, create a new mesh with its Vertex and Index buffer.
			if(sMesh::HasMesh(MeshData->ID))
			{
				RenderObjectData.pMesh = sMesh::GetMesh(MeshData->ID);
			}
			else
			{
				// TODO: This is not being added to the meshes map?
				RenderObjectData.pMesh = new sMesh(MeshData->ID);
				RenderObjectData.pMesh->NumIndices = static_cast<uint32_t>(MeshData->Indices32.size());
				vkutils::CreateVertexBuffer(m_pVulkanDevice, MeshData->Vertices, RenderObjectData.pMesh->VertexBuffer);
				vkutils::CreateIndexBuffer(m_pVulkanDevice, MeshData->Indices32, RenderObjectData.pMesh->IndexBuffer);
			}
		}

		m_RenderObjectsData.push_back(RenderObjectData);
	}

	void* Data;
	vmaMapMemory(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation, &Data);

	sGPURenderObjectData* GPURenderObjectData = static_cast<sGPURenderObjectData*>(Data);

	for (size_t i = 0; i < aRenderObjects.size(); ++i)
	{
		GPURenderObjectData[i].ModelMatrix = aRenderObjects[i]->ModelMatrix;
	}

	vmaUnmapMemory(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation);

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->HandleSceneChanged();
	}
}

void CVulkanBackend::InitCommandPools()
{
	const VkDevice Device = m_pVulkanDevice->m_Device;
	// Creation of command structures
	VkCommandPoolCreateInfo CommandPoolInfo = vkinit::CommandPoolCreateInfo(m_pVulkanDevice->m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(m_pVulkanDevice->m_Device, &CommandPoolInfo, nullptr, &m_CommandPool));

	for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_CommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(Device, &CmdAllocInfo, &m_FramesData[i].MainCommandBuffer));
	}

	// TODO: Move to CVulkanDevice.
	VkCommandPoolCreateInfo UploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(m_pVulkanDevice->m_GraphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(Device, &UploadCommandPoolInfo, nullptr, &m_pVulkanDevice->m_UploadContext.m_CommandPool));

	m_pVulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyCommandPool(Device, m_CommandPool, nullptr);
		vkDestroyCommandPool(Device, m_pVulkanDevice->m_UploadContext.m_CommandPool, nullptr);
	});
}

void CVulkanBackend::InitSyncStructures()
{
	const VkDevice Device = m_pVulkanDevice->m_Device;

	VkSemaphoreCreateInfo SemaphoreInfo = vkinit::SemaphoreCreateInfo();

	for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VK_CHECK(vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_FramesData[i].PresentSemaphore));
		VK_CHECK(vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_FramesData[i].RenderSemaphore));

		VkFenceCreateInfo FenceInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		VK_CHECK(vkCreateFence(Device, &FenceInfo, nullptr, &m_FramesData[i].RenderFence));
	}

	// TODO: Move to CVulkanDevice.
	VkFenceCreateInfo UploadFenceInfo = vkinit::FenceCreateInfo();
	VK_CHECK(vkCreateFence(Device, &UploadFenceInfo, nullptr, &m_pVulkanDevice->m_UploadContext.m_UploadFence));

	m_pVulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vkDestroySemaphore(Device, m_FramesData[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(Device, m_FramesData[i].RenderSemaphore, nullptr);
			vkDestroyFence(Device, m_FramesData[i].RenderFence, nullptr);
			vkDestroyFence(Device, m_pVulkanDevice->m_UploadContext.m_UploadFence, nullptr);
		}
	});
}

void CVulkanBackend::InitTextureSamplers()
{
	VkPhysicalDeviceProperties Properties = {};
	vkGetPhysicalDeviceProperties(m_pVulkanDevice->m_PhysicalDevice, &Properties);

	VkSamplerCreateInfo SamplerInfo = {};
	SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInfo.magFilter = VK_FILTER_LINEAR;
	SamplerInfo.minFilter = VK_FILTER_LINEAR;
	SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.anisotropyEnable = VK_FALSE;
	SamplerInfo.maxAnisotropy = 1.0f; //Properties.limits.maxSamplerAnisotropy;
	SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerInfo.unnormalizedCoordinates = VK_FALSE;
	SamplerInfo.compareEnable = VK_FALSE;
	SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VK_CHECK(vkCreateSampler(m_pVulkanDevice->m_Device, &SamplerInfo, nullptr, &m_DefaultSampler));

	m_pVulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroySampler(m_pVulkanDevice->m_Device, m_DefaultSampler, nullptr);
	});
}

void CVulkanBackend::InitDescriptorSetPool()
{
	VkDescriptorPoolSize UBOPoolSize = {};
	UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOPoolSize.descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

	VkDescriptorPoolSize SamplerPoolSize = {};
	SamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerPoolSize.descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

	VkDescriptorPoolSize ObjectsDataPoolSize = {};
	ObjectsDataPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ObjectsDataPoolSize.descriptorCount = 1;

	std::array<VkDescriptorPoolSize, 3> PoolSizes = { UBOPoolSize, SamplerPoolSize, ObjectsDataPoolSize };

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
	PoolInfo.pPoolSizes = PoolSizes.data();;
	PoolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP) + 1; // +1 set for the objects descriptor set.

	VK_CHECK(vkCreateDescriptorPool(m_pVulkanDevice->m_Device, &PoolInfo, nullptr, &m_DescriptorPool));

	m_pVulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyDescriptorPool(m_pVulkanDevice->m_Device, m_DescriptorPool, nullptr);
	});
}

void CVulkanBackend::InitDescriptorSets()
{
	VkDescriptorSetAllocateInfo AllocInfo{};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.descriptorPool = m_DescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &m_DescriptorSetLayout;

	for (size_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VK_CHECK(vkAllocateDescriptorSets(m_pVulkanDevice->m_Device, &AllocInfo, &m_FramesData[i].DescriptorSet));
	}

	for (size_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_FramesData[i].UBOBuffer.Buffer;
		BufferInfo.offset = 0;
		BufferInfo.range = sizeof(sFrameUBO);

		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageInfo.imageView = m_ImageView;
		ImageInfo.sampler = m_DefaultSampler;

		std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

		DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[0].dstSet =  m_FramesData[i].DescriptorSet;
		DescriptorWrites[0].dstBinding = 0;
		DescriptorWrites[0].dstArrayElement = 0;
		DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWrites[0].descriptorCount = 1;
		DescriptorWrites[0].pBufferInfo = &BufferInfo;
		DescriptorWrites[0].pImageInfo = nullptr;
		DescriptorWrites[0].pTexelBufferView = nullptr;

		DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[1].dstSet = m_FramesData[i].DescriptorSet;
		DescriptorWrites[1].dstBinding = 1;
		DescriptorWrites[1].dstArrayElement = 0;
		DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorWrites[1].descriptorCount = 1;
		DescriptorWrites[1].pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}

	VkDescriptorSetAllocateInfo RenderObjectsAllocInfo{};
	RenderObjectsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	RenderObjectsAllocInfo.descriptorPool = m_DescriptorPool;
	RenderObjectsAllocInfo.descriptorSetCount = 1;
	RenderObjectsAllocInfo.pSetLayouts = &m_RenderObjectsSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(m_pVulkanDevice->m_Device, &RenderObjectsAllocInfo, &m_ObjectsDataDescriptorSet));

	VkDescriptorBufferInfo RenderObjectsBufferInfo = {};
	RenderObjectsBufferInfo.buffer = m_ObjectsDataBuffer.Buffer;
	RenderObjectsBufferInfo.offset = 0;
	RenderObjectsBufferInfo.range = sizeof(sGPURenderObjectData) * MAX_RENDER_OBJECTS;

	VkWriteDescriptorSet RenderObjectsDescriptorWrite{};
	RenderObjectsDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	RenderObjectsDescriptorWrite.dstSet = m_ObjectsDataDescriptorSet;
	RenderObjectsDescriptorWrite.dstBinding = 0;
	RenderObjectsDescriptorWrite.dstArrayElement = 0;
	RenderObjectsDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	RenderObjectsDescriptorWrite.descriptorCount = 1;
	RenderObjectsDescriptorWrite.pBufferInfo = &RenderObjectsBufferInfo;
	RenderObjectsDescriptorWrite.pImageInfo = nullptr;
	RenderObjectsDescriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, 1, &RenderObjectsDescriptorWrite, 0, nullptr);
}

void CVulkanBackend::InitDescriptorSetLayouts()
{
	// FRAME DESCRIPTOR LAYOUT CREATION.
	VkDescriptorSetLayoutBinding FrameUBOLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkDescriptorSetLayoutBinding SamplerLayoutBinding =  vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	std::array<VkDescriptorSetLayoutBinding, 2> Bindings = { FrameUBOLayoutBinding, SamplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
	LayoutInfo.pBindings = Bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_DescriptorSetLayout));

	VkDeviceSize BufferSize = sizeof(sFrameUBO);
	for (int i = 0; i < FRAME_OVERLAP; ++i)
	{
		m_FramesData[i].UBOBuffer = vkutils::CreateBuffer(m_pVulkanDevice, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		vmaMapMemory(m_pVulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Allocation, &m_FramesData[i].MappedUBOBuffer);
	}

	// RENDER OBJECTS DESCRIPTOR LAYOUT CREATION.
	VkDescriptorSetLayoutBinding RenderObjectsLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo RenderObjectsLayoutInfo = {};
	RenderObjectsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	RenderObjectsLayoutInfo.bindingCount = 1;
	RenderObjectsLayoutInfo.pBindings = &RenderObjectsLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &RenderObjectsLayoutInfo, nullptr, &m_RenderObjectsSetLayout));

	m_ObjectsDataBuffer = vkutils::CreateBuffer(m_pVulkanDevice, sizeof(sGPURenderObjectData) * MAX_RENDER_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkDescriptorSetLayoutBinding SingleTextureLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	VkDescriptorSetLayoutCreateInfo SingleTextureLayoutInfo = {};
	SingleTextureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	SingleTextureLayoutInfo.bindingCount = 1;
	SingleTextureLayoutInfo.pBindings = &SingleTextureLayoutBinding;

	m_pVulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		for (size_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vmaDestroyBuffer(m_pVulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Buffer, m_FramesData[i].UBOBuffer.Allocation);
		}

		vmaDestroyBuffer(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Buffer, m_ObjectsDataBuffer.Allocation);

		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_RenderObjectsSetLayout, nullptr);
	});
}

IRenderPath* CVulkanBackend::CreateRenderPath()
{
	// TODO: I need a module getter.
	const eRenderPath RenderPath = CEngine::Get()->GetRenderModule()->GetRenderPath();
	IRenderPath* NewRenderPath = nullptr;
	switch (RenderPath)
	{
		case eRenderPath::FORWARD:
		{
			NewRenderPath = new CVulkanForwardRenderPath(this, m_pVulkanDevice, m_pVulkanSwapchain);
		}
		break;
		
		case eRenderPath::DEFERRED:
		{
			NewRenderPath = new CVulkanDeferredRenderPath(this, m_pVulkanDevice, m_pVulkanSwapchain);
		}
		break;

		default:
			SGSERROR("No IRenderPath created!");
			break;
	}

	InitRenderPath(NewRenderPath);

	return NewRenderPath;
}

void CVulkanBackend::InitRenderPath(IRenderPath* aRenderPath)
{
	if (aRenderPath == nullptr)
		return;

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->DestroyResources();
		delete m_pCurrentRenderPath;
		m_pCurrentRenderPath = nullptr;
	}

	m_pCurrentRenderPath = aRenderPath;
	m_pCurrentRenderPath->CreateResources();
	m_pCurrentRenderPath->HandleSceneChanged();
}

void CVulkanBackend::UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx)
{
	assert(ImageIdx >= 0 && ImageIdx < FRAME_OVERLAP);

	sFrameUBO FrameUBO = {};
	FrameUBO.View = aCamera->GetViewMatrix();
	FrameUBO.Proj = glm::perspective(glm::radians(70.0f), m_pVulkanSwapchain->m_WindowExtent.width / (float)m_pVulkanSwapchain->m_WindowExtent.height, 0.1f, 200.0f);
	FrameUBO.Proj[1][1] *= -1;
	FrameUBO.ViewProj = FrameUBO.Proj * FrameUBO.View;

	memcpy(m_FramesData[ImageIdx].MappedUBOBuffer, &FrameUBO, sizeof(sFrameUBO));
}

bool CVulkanBackend::HasStencilComponent(VkFormat aFormat)
{
	return aFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || aFormat == VK_FORMAT_D24_UNORM_S8_UINT;
}
