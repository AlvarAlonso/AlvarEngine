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
#include <renderer/resources/material.hpp>
#include <renderer/resources/texture.hpp>
#include "resources/vk_texture.hpp"

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

	vkutils::LoadImageFromFile(m_pVulkanDevice, "../Resources/Images/viking_room.png", m_Image);
	m_MainDeletionQueue.PushFunction([=]
	{
		vmaDestroyImage(m_pVulkanDevice->m_Allocator, m_Image.Image, m_Image.Allocation);
	});

	// TODO: Placeholder for testing purposes. TO BE REMOVED.
	VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(m_pVulkanDevice->m_Device, &ViewInfo, nullptr, &m_ImageView));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyImageView(m_pVulkanDevice->m_Device, m_ImageView, nullptr);
	});

	InitDescriptorSets();

	m_pCurrentRenderPath = CreateRenderPath();

    m_bIsInitialized = true;

    return true;
}

void CVulkanBackend::Render(const CCamera* const aCamera)
{
	assert(m_bIsInitialized);

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->UpdateBuffers();
		m_pCurrentRenderPath->Render(aCamera);
	}
}

bool CVulkanBackend::Shutdown()
{
	SGSINFO("Shutting down Vulkan");
	vkQueueWaitIdle(m_pVulkanDevice->m_GraphicsQueue);

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->DestroyResources();
		delete m_pCurrentRenderPath;
	}

	for (auto& MaterialDescriptor : m_MaterialDescriptors)
	{
		delete MaterialDescriptor.second;
	}
	m_MaterialDescriptors.clear();

	// Destroy CVulkanBackend's vulkan resources.
	m_MainDeletionQueue.Flush();

	if (m_pVulkanSwapchain)
	{
		// Destructor automatically cleans resources.
		delete m_pVulkanSwapchain;
	}

	// Destructor automatically cleans resources.
	delete m_pVulkanDevice;

    return false;
}

void CVulkanBackend::HandleWindowResize()
{
	m_bWasWindowResized = true;
}

void CVulkanBackend::CreateRenderablesData(const std::vector<CRenderable*>& aRenderables)
{
	m_Renderables.reserve(aRenderables.size());
	for (const auto& Renderable : aRenderables)
	{
		CVulkanRenderable* pVulkanRenderable = dynamic_cast<CVulkanRenderable*>(Renderable);
		if (pVulkanRenderable)
		{
			m_Renderables.emplace_back(pVulkanRenderable);
		}
		else
		{
			SGSERROR("The renderables were not of type CVulkanRenderable!!!");
			std::abort();
		}
	}

	// TODO: Triple for dona nauseas.
	for (const auto& Renderable : aRenderables)
	{
		// TODO: Use namespaces because otherwise is very confusing to know if I'm dealing with vulkan types or generic render types.

		// Create the material descriptors.
		for (const auto& MeshNode : Renderable->m_pRoots)
		{
			for (const auto& SubMesh : MeshNode->m_pMeshData->SubMeshes)
			{
				CMaterial* pMaterial = SubMesh->m_Material;
				if (SubMesh->m_Material == nullptr)
				{
					SGSWARN("The SubMesh from the MeshNode %s does not have a material. Using default material.", MeshNode->m_Name.c_str());
					pMaterial = CMaterial::Get("default_material");
				}

				assert(pMaterial);

				sMaterialDescriptor* MaterialDescriptor = new sMaterialDescriptor();
				MaterialDescriptor->pMaterial = pMaterial;

				const sMaterialProperties Props = MaterialDescriptor->pMaterial->GetMaterialProperties();

				MaterialDescriptor->ConstantsBuffer = vkutils::CreateBuffer(m_pVulkanDevice, sizeof(sMaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

				void* Data;
				vmaMapMemory(m_pVulkanDevice->m_Allocator, MaterialDescriptor->ConstantsBuffer.Allocation, &Data);
				memcpy(Data, &Props.MaterialConstants, sizeof(sMaterialConstants));
				vmaUnmapMemory(m_pVulkanDevice->m_Allocator, MaterialDescriptor->ConstantsBuffer.Allocation);

				CVkTexture* pAlbedoTexture = nullptr;
				CVkTexture* pMetalRoughnessTexture = nullptr;
				CVkTexture* pEmissiveTexture = nullptr;
				CVkTexture* pNormalTexture = nullptr;

				// TODO: Refactor this into a function where, in case of nullptr, it places a default texture.
				if (Props.pAlbedoTexture)
				{
					pAlbedoTexture = CTexture::Get<CVkTexture>(Props.pAlbedoTexture->GetFilename());
				}
				if (Props.pMetallicRoughnessTexture)
				{
					pMetalRoughnessTexture = CTexture::Get<CVkTexture>(Props.pMetallicRoughnessTexture->GetFilename());
				}
				if (Props.pEmissiveTexture)
				{
					pEmissiveTexture = CTexture::Get<CVkTexture>(Props.pEmissiveTexture->GetFilename());
				}
				if (Props.pNormalTexture)
				{
					pNormalTexture = CTexture::Get<CVkTexture>(Props.pNormalTexture->GetFilename());
				}
				
				MaterialDescriptor->Resources.pAlbedoTexture = pAlbedoTexture;
				MaterialDescriptor->Resources.pMetalRoughnessTexture = pMetalRoughnessTexture;
				MaterialDescriptor->Resources.pEmissiveTexture = pEmissiveTexture;
				MaterialDescriptor->Resources.pNormalTexture = pNormalTexture;

				m_MaterialDescriptors.insert({pMaterial->GetID(), MaterialDescriptor});
				// Descriptors will be created in the specific function to create descriptors.
			}
		}
	}

	CreateSceneDescriptorSets();

	void* Data;
	vmaMapMemory(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation, &Data);

	// There will be a draw call per CMeshNode, hence, we need a transform for each CMeshNode.
	sGPURenderObjectData* GPURenderObjectData = static_cast<sGPURenderObjectData*>(Data);
	size_t Index = 0;

	for (const auto& Renderable : aRenderables)
	{
		for (const auto& Root : Renderable->m_pRoots)
		{
			AddTransformsToBuffer(GPURenderObjectData, Index, Root);
		}
	}

	vmaUnmapMemory(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation);

	if (m_pCurrentRenderPath)
	{
		m_pCurrentRenderPath->HandleSceneChanged();
	}
}

void CVulkanBackend::ChangeRenderPath()
{
	m_pCurrentRenderPath = CreateRenderPath();
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

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyCommandPool(Device, m_CommandPool, nullptr);
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

	m_MainDeletionQueue.PushFunction([=]
	{
		for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vkDestroySemaphore(Device, m_FramesData[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(Device, m_FramesData[i].RenderSemaphore, nullptr);
			vkDestroyFence(Device, m_FramesData[i].RenderFence, nullptr);
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

	m_MainDeletionQueue.PushFunction([=]
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
	PoolInfo.pPoolSizes = PoolSizes.data();
	// TODO: This does not make sense. The COMBINE_IMAGE_SAMPLER type is not accounted for.
	PoolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP) + 1; // +1 set for the objects descriptor set.
	
	VK_CHECK(vkCreateDescriptorPool(m_pVulkanDevice->m_Device, &PoolInfo, nullptr, &m_DescriptorPool));


	VkDescriptorPoolSize MaterialTexturesPoolSize = {};
	MaterialTexturesPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	MaterialTexturesPoolSize.descriptorCount = 4 * MAX_RENDER_OBJECTS; // Albedo, MetalRoughness, Emissive and Normal for a total of 4 textures.

	VkDescriptorPoolSize MaterialConstantsPoolSize = {};
	MaterialConstantsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	MaterialTexturesPoolSize.descriptorCount = MAX_RENDER_OBJECTS;

	std::array<VkDescriptorPoolSize, 2> MaterialPoolSizes = { MaterialTexturesPoolSize, MaterialConstantsPoolSize };

	VkDescriptorPoolCreateInfo MaterialPoolInfo = {};
	MaterialPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	MaterialPoolInfo.poolSizeCount = static_cast<uint32_t>(MaterialPoolSizes.size());
	MaterialPoolInfo.pPoolSizes = MaterialPoolSizes.data();
	MaterialPoolInfo.maxSets = MAX_RENDER_OBJECTS;

	VK_CHECK(vkCreateDescriptorPool(m_pVulkanDevice->m_Device, &MaterialPoolInfo, nullptr, &m_MaterialsPool));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyDescriptorPool(m_pVulkanDevice->m_Device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorPool(m_pVulkanDevice->m_Device, m_MaterialsPool, nullptr);
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
		BufferInfo.range = sizeof(sCameraFrameUBO);

		VkWriteDescriptorSet Write = {};
		Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Write.dstSet = m_FramesData[i].DescriptorSet;
		Write.dstBinding = 0;
		Write.dstArrayElement = 0;
		Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Write.descriptorCount = 1;
		Write.pBufferInfo = &BufferInfo;
		Write.pImageInfo = nullptr;
		Write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, 1,&Write, 0, nullptr);
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
	VkDescriptorSetLayoutBinding FrameUBOVertLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutInfo.bindingCount = 1;
	LayoutInfo.pBindings = &FrameUBOVertLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_DescriptorSetLayout));

	VkDeviceSize BufferSize = sizeof(sCameraFrameUBO);
	for (int i = 0; i < FRAME_OVERLAP; ++i)
	{
		m_FramesData[i].UBOBuffer = vkutils::CreateBuffer(m_pVulkanDevice, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		vmaMapMemory(m_pVulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Allocation, &m_FramesData[i].MappedUBOBuffer);
	}

	// RENDER OBJECTS DESCRIPTOR LAYOUT CREATION.
	VkDescriptorSetLayoutBinding TransformLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);


	VkDescriptorSetLayoutCreateInfo RenderObjectsLayoutInfo = {};
	RenderObjectsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	RenderObjectsLayoutInfo.bindingCount = 1;
	RenderObjectsLayoutInfo.pBindings = &TransformLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &RenderObjectsLayoutInfo, nullptr, &m_RenderObjectsSetLayout));

	// TODO: Buffer creation should not be here.
	m_ObjectsDataBuffer = vkutils::CreateBuffer(m_pVulkanDevice, sizeof(sGPURenderObjectData) * MAX_RENDER_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// Material Layout Binding.
	VkDescriptorSetLayoutBinding MaterialConstants = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding AlbedoLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding MetalRoughnessLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutBinding EmissiveLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	VkDescriptorSetLayoutBinding NormalLayoutBinding = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
	
	const std::array<VkDescriptorSetLayoutBinding, 4> MaterialLayoutBindings = { MaterialConstants, AlbedoLayoutBinding, MetalRoughnessLayoutBinding, NormalLayoutBinding };

	VkDescriptorSetLayoutCreateInfo MaterialLayoutInfo = {};
	MaterialLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	MaterialLayoutInfo.bindingCount = static_cast<uint32_t>(MaterialLayoutBindings.size());
	MaterialLayoutInfo.pBindings = MaterialLayoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &MaterialLayoutInfo, nullptr, &m_MaterialsSetLayout));

	m_MainDeletionQueue.PushFunction([=]
	{
		for (size_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vmaUnmapMemory(m_pVulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Allocation);
			vmaDestroyBuffer(m_pVulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Buffer, m_FramesData[i].UBOBuffer.Allocation);
		}

		vmaDestroyBuffer(m_pVulkanDevice->m_Allocator, m_ObjectsDataBuffer.Buffer, m_ObjectsDataBuffer.Allocation);

		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_RenderObjectsSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_MaterialsSetLayout, nullptr);
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

void CVulkanBackend::CreateSceneDescriptorSets()
{
	VkDescriptorSetAllocateInfo MaterialsAllocInfo = {};
	MaterialsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	MaterialsAllocInfo.descriptorPool = m_MaterialsPool;
	MaterialsAllocInfo.descriptorSetCount = 1;
	MaterialsAllocInfo.pSetLayouts = &m_MaterialsSetLayout;
	
	// TODO: Maybe a unique descriptor with ALL the materials and each mesh reference them by an index?
	for (auto& MaterialDescriptorTuple : m_MaterialDescriptors)
	{
		auto& MaterialDescriptor = MaterialDescriptorTuple.second;

		VK_CHECK(vkAllocateDescriptorSets(m_pVulkanDevice->m_Device, &MaterialsAllocInfo, &MaterialDescriptor->DescriptorSet));
	
		const sMaterialResources& Resources = MaterialDescriptor->Resources;

		VkDescriptorBufferInfo ConstantsBufferInfo = {};
		ConstantsBufferInfo.buffer = MaterialDescriptor->ConstantsBuffer.Buffer;
		ConstantsBufferInfo.offset = 0;
		ConstantsBufferInfo.range = sizeof(sMaterialConstants);

		VkDescriptorImageInfo AlbedoImageInfo = {};
		AlbedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		AlbedoImageInfo.imageView = Resources.pAlbedoTexture->GetImageView();
		AlbedoImageInfo.sampler = m_DefaultSampler;

		VkDescriptorImageInfo MetalRoughnessImageInfo = {};
		MetalRoughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		MetalRoughnessImageInfo.imageView = Resources.pMetalRoughnessTexture->GetImageView();
		MetalRoughnessImageInfo.sampler = m_DefaultSampler;

		VkDescriptorImageInfo EmissiveImageInfo = {};
		EmissiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		EmissiveImageInfo.imageView = Resources.pEmissiveTexture->GetImageView();
		EmissiveImageInfo.sampler = m_DefaultSampler;

		VkDescriptorImageInfo NormalImageInfo = {};
		NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		NormalImageInfo.imageView = Resources.pNormalTexture->GetImageView();
		NormalImageInfo.sampler = m_DefaultSampler;

		std::array<VkDescriptorImageInfo, 3> DescriptorImageInfos = { AlbedoImageInfo, MetalRoughnessImageInfo, NormalImageInfo };

		VkWriteDescriptorSet MaterialImagesDescriptorWrite = {};
		MaterialImagesDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		MaterialImagesDescriptorWrite.dstSet = MaterialDescriptor->DescriptorSet;
		MaterialImagesDescriptorWrite.dstBinding = 1;
		MaterialImagesDescriptorWrite.dstArrayElement = 0;
		MaterialImagesDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		MaterialImagesDescriptorWrite.descriptorCount = static_cast<uint32_t>(DescriptorImageInfos.size()); // Add one for the buffer descriptor.
		MaterialImagesDescriptorWrite.pBufferInfo = nullptr;
		MaterialImagesDescriptorWrite.pImageInfo = DescriptorImageInfos.data();
		MaterialImagesDescriptorWrite.pTexelBufferView = nullptr;

		VkWriteDescriptorSet MaterialConstantsDescriptorWrite = {};
		MaterialConstantsDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		MaterialConstantsDescriptorWrite.dstSet = MaterialDescriptor->DescriptorSet;
		MaterialConstantsDescriptorWrite.dstBinding = 0;
		MaterialConstantsDescriptorWrite.dstArrayElement = 0;
		MaterialConstantsDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		MaterialConstantsDescriptorWrite.descriptorCount = 1; // Add one for the buffer descriptor.
		MaterialConstantsDescriptorWrite.pBufferInfo = &ConstantsBufferInfo;
		MaterialConstantsDescriptorWrite.pImageInfo = nullptr;
		MaterialConstantsDescriptorWrite.pTexelBufferView = nullptr;

		std::array<VkWriteDescriptorSet, 2> Writes = { MaterialImagesDescriptorWrite, MaterialConstantsDescriptorWrite };

		vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, static_cast<uint32_t>(Writes.size()), Writes.data(), 0, nullptr);
	}
}

void CVulkanBackend::UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx)
{
	assert(ImageIdx >= 0 && ImageIdx < FRAME_OVERLAP);

	sCameraFrameUBO FrameUBO = {};
	FrameUBO.View = aCamera->GetViewMatrix();
	// TODO: Do not hardcode this.
	FrameUBO.Proj = glm::perspective(glm::radians(70.0f), m_pVulkanSwapchain->m_WindowExtent.width / (float)m_pVulkanSwapchain->m_WindowExtent.height, 0.1f, 200.0f);
	FrameUBO.Proj[1][1] *= -1;
	FrameUBO.ViewProj = FrameUBO.Proj * FrameUBO.View;
	FrameUBO.Pos = aCamera->GetPosition();

	memcpy(m_FramesData[ImageIdx].MappedUBOBuffer, &FrameUBO, sizeof(sCameraFrameUBO));
}

bool CVulkanBackend::HasStencilComponent(VkFormat aFormat)
{
	return aFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || aFormat == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CVulkanBackend::AddTransformsToBuffer(sGPURenderObjectData* apBuffer, size_t& aIndex, CMeshNode* apMeshNode)
{
	for (const auto& MeshNode : apMeshNode->m_Children)
	{
		AddTransformsToBuffer(apBuffer, aIndex, MeshNode);
	}

	apBuffer[aIndex].ModelMatrix = apMeshNode->GetWorldTransform();
	++aIndex;
}
