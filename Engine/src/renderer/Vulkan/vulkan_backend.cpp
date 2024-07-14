#include "vulkan_backend.hpp"

#define GLM_FORCE_RADIANS

#include "engine.hpp"
#include "core/defines.h"
#include "core/logger.h"
#include "vk_types.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"
#include "vulkan_device.hpp"
#include <renderer/core/camera.hpp>

#include <VulkanBootstrap/VkBootstrap.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>
#include <array>

sVertexInputDescription sVertex::GetVertexDescription()
{
	sVertexInputDescription Description{};

	VkVertexInputBindingDescription MainBinding = {};
	MainBinding.binding = 0;
	MainBinding.stride = sizeof(sVertex);
	MainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Description.Bindings.push_back(MainBinding);

	VkVertexInputAttributeDescription PositionAttribute = {};
	PositionAttribute.binding = 0;
	PositionAttribute.location = 0;
	PositionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	PositionAttribute.offset = offsetof(sVertex, Position);

	VkVertexInputAttributeDescription ColorAttribute = {};
	ColorAttribute.binding = 0;
	ColorAttribute.location = 1;
	ColorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	ColorAttribute.offset = offsetof(sVertex, Color);

	VkVertexInputAttributeDescription UVAttribute = {};
	UVAttribute.binding = 0;
	UVAttribute.location = 2;
	UVAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	UVAttribute.offset = offsetof(sVertex, UV);

	Description.Attributes.push_back(PositionAttribute);
	Description.Attributes.push_back(ColorAttribute);
	Description.Attributes.push_back(UVAttribute);

	return Description;
}

CVulkanBackend::CVulkanBackend() :
	m_bIsInitialized(false),
	m_ImageIdx(0),
	m_CurrentFrame(0),
	m_bWasWindowResized(false)
{
}

bool CVulkanBackend::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
	m_VulkanDevice = new CVulkanDevice();
	m_VulkanDevice->InitVulkanDevice(aInstanceHandle, aWindowHandle);

	InitSwapchain();

	InitDepthBuffer();

	InitCommandPools();

	InitRenderPass();

	InitFramebuffers();

	InitDescriptorSetLayouts();

	InitDescriptorSetPool();

	InitPipelines();

	InitSyncStructures();

	InitTextureSamplers();

	vkutils::LoadImageFromFile(m_VulkanDevice, "../Resources/Images/viking_room.png", m_Image);
	
	// TODO: Placeholder for testing purposes. TO BE REMOVED.
	VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(m_VulkanDevice->m_Device, &ViewInfo, nullptr, &m_ImageView));

	InitDescriptorSets();

    m_bIsInitialized = true;

    return true;
}

void CVulkanBackend::Render(const CCamera* const aCamera)
{
	assert(m_bIsInitialized);

	RenderFrame(aCamera);
}

bool CVulkanBackend::Shutdown()
{
	if (m_bIsInitialized)
	{
		SGSINFO("Shutting down Vulkan");

		vkDestroyCommandPool(m_VulkanDevice->m_Device, m_CommandPool, nullptr);

		vkDestroySwapchainKHR(m_VulkanDevice->m_Device, m_Swapchain, nullptr);

		vkDestroyRenderPass(m_VulkanDevice->m_Device, m_RenderPass, nullptr);

		for (int32 i = 0; i < m_SwapchainImageViews.size(); ++i)
		{
			vkDestroyFramebuffer(m_VulkanDevice->m_Device, m_Framebuffers[i], nullptr);
			vkDestroyImageView(m_VulkanDevice->m_Device, m_SwapchainImageViews[i], nullptr);
		}
	}

	delete m_VulkanDevice;

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
		RenderObjectData.pMesh = sMesh::GetMesh(RenderObject->pRenderObjectInfo->MeshPath);
		m_RenderObjectsData.push_back(RenderObjectData);
	}

	for (const auto& RenderObjectData : m_RenderObjectsData)
	{
		vkutils::CreateVertexBuffer(m_VulkanDevice, RenderObjectData.pMesh->Vertices, RenderObjectData.pMesh->VertexBuffer);
		vkutils::CreateIndexBuffer(m_VulkanDevice, RenderObjectData.pMesh->Indices, RenderObjectData.pMesh->IndexBuffer);
	}

	void* Data;
	vmaMapMemory(m_VulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation, &Data);

	sGPURenderObjectData* GPURenderObjectData = static_cast<sGPURenderObjectData*>(Data);

	for (size_t i = 0; i < aRenderObjects.size(); ++i)
	{
		GPURenderObjectData[i].ModelMatrix = aRenderObjects[i]->ModelMatrix;
	}

	vmaUnmapMemory(m_VulkanDevice->m_Allocator, m_ObjectsDataBuffer.Allocation);
}

void CVulkanBackend::InitSwapchain()
{
	sDimension2D WindowDimensions = Engine::Get()->GetAppWindowDimensions();
	m_WindowExtent = {WindowDimensions.Width, WindowDimensions.Height};

	SGSINFO("Creating Swapchain. Framebuffer Size: %d, %d.", WindowDimensions.Width, WindowDimensions.Height);

	// TODO: initialize extent according to Window's created window
	// Swapchain initialization.
	vkb::SwapchainBuilder SwapchainBuilder{m_VulkanDevice->m_PhysicalDevice, m_VulkanDevice->m_Device, m_VulkanDevice->m_Surface};
	vkb::Swapchain vkbSwapchain = SwapchainBuilder
	.use_default_format_selection()
	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	.set_desired_extent(m_WindowExtent.width, m_WindowExtent.height)
	.build()
	.value();

	m_Swapchain = vkbSwapchain.swapchain;
	m_SwapchainImageFormat = vkbSwapchain.image_format;
	m_SwapchainImages = vkbSwapchain.get_images().value();
	m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void CVulkanBackend::InitCommandPools()
{
	const VkDevice Device = m_VulkanDevice->m_Device;
	// Creation of command structures
	VkCommandPoolCreateInfo CommandPoolInfo = vkinit::CommandPoolCreateInfo(m_VulkanDevice->m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(m_VulkanDevice->m_Device, &CommandPoolInfo, nullptr, &m_CommandPool));

	for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_CommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(Device, &CmdAllocInfo, &m_FramesData[i].MainCommandBuffer));
	}

	// TODO: Move to CVulkanDevice.
	VkCommandPoolCreateInfo UploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(m_VulkanDevice->m_GraphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(Device, &UploadCommandPoolInfo, nullptr, &m_VulkanDevice->m_UploadContext.m_CommandPool));

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyCommandPool(Device, m_CommandPool, nullptr);
		vkDestroyCommandPool(Device, m_VulkanDevice->m_UploadContext.m_CommandPool, nullptr);
	});
}

void CVulkanBackend::InitDepthBuffer()
{
	VkFormat DepthFormat = FindDepthFormat();

	VkImageCreateInfo DepthImgInfo = vkinit::ImageCreateInfo(DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, {m_WindowExtent.width, m_WindowExtent.height, 1});

	VmaAllocationCreateInfo DepthImgAllocInfo = {};
	DepthImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	DepthImgAllocInfo.requiredFlags = VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_VulkanDevice->m_Allocator, &DepthImgInfo, &DepthImgAllocInfo, &m_DepthImage.Image, &m_DepthImage.Allocation, nullptr);

	VkImageViewCreateInfo DepthViewInfo = vkinit::ImageViewCreateInfo(DepthFormat, m_DepthImage.Image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(m_VulkanDevice->m_Device, &DepthViewInfo, nullptr, &m_DepthImageView));
}

void CVulkanBackend::InitSyncStructures()
{
	const VkDevice Device = m_VulkanDevice->m_Device;

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
	VK_CHECK(vkCreateFence(Device, &UploadFenceInfo, nullptr, &m_VulkanDevice->m_UploadContext.m_UploadFence));

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vkDestroySemaphore(Device, m_FramesData[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(Device, m_FramesData[i].RenderSemaphore, nullptr);
			vkDestroyFence(Device, m_FramesData[i].RenderFence, nullptr);
			vkDestroyFence(Device, m_VulkanDevice->m_UploadContext.m_UploadFence, nullptr);
		}
	});
}

void CVulkanBackend::InitTextureSamplers()
{
	VkPhysicalDeviceProperties Properties = {};
	vkGetPhysicalDeviceProperties(m_VulkanDevice->m_PhysicalDevice, &Properties);

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

	VK_CHECK(vkCreateSampler(m_VulkanDevice->m_Device, &SamplerInfo, nullptr, &m_DefaultSampler));

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroySampler(m_VulkanDevice->m_Device, m_DefaultSampler, nullptr);
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

	VK_CHECK(vkCreateDescriptorPool(m_VulkanDevice->m_Device, &PoolInfo, nullptr, &m_DescriptorPool));

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyDescriptorPool(m_VulkanDevice->m_Device, m_DescriptorPool, nullptr);
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
		VK_CHECK(vkAllocateDescriptorSets(m_VulkanDevice->m_Device, &AllocInfo, &m_FramesData[i].DescriptorSet));
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

		vkUpdateDescriptorSets(m_VulkanDevice->m_Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}

	VkDescriptorSetAllocateInfo RenderObjectsAllocInfo{};
	RenderObjectsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	RenderObjectsAllocInfo.descriptorPool = m_DescriptorPool;
	RenderObjectsAllocInfo.descriptorSetCount = 1;
	RenderObjectsAllocInfo.pSetLayouts = &m_RenderObjectsSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(m_VulkanDevice->m_Device, &RenderObjectsAllocInfo, &m_ObjectsDataDescriptorSet));

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

	vkUpdateDescriptorSets(m_VulkanDevice->m_Device, 1, &RenderObjectsDescriptorWrite, 0, nullptr);
}

void CVulkanBackend::InitDescriptorSetLayouts()
{
	// FRAME DESCRIPTOR LAYOUT CREATION.
	VkDescriptorSetLayoutBinding FrameUBOLayoutBinding = {};
	FrameUBOLayoutBinding.binding = 0;
	FrameUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	FrameUBOLayoutBinding.descriptorCount = 1;
	FrameUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	FrameUBOLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
	SamplerLayoutBinding.binding = 1;
	SamplerLayoutBinding.descriptorCount = 1;
	SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerLayoutBinding.pImmutableSamplers = nullptr;
	SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> Bindings = { FrameUBOLayoutBinding, SamplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
	LayoutInfo.pBindings = Bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_VulkanDevice->m_Device, &LayoutInfo, nullptr, &m_DescriptorSetLayout));

	VkDeviceSize BufferSize = sizeof(sFrameUBO);
	for (int i = 0; i < FRAME_OVERLAP; ++i)
	{
		m_FramesData[i].UBOBuffer = vkutils::CreateBuffer(m_VulkanDevice, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		vmaMapMemory(m_VulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Allocation, &m_FramesData[i].MappedUBOBuffer);
	}

	// RENDER OBJECTS DESCRIPTOR LAYOUT CREATION.
	VkDescriptorSetLayoutBinding RenderObjectsLayoutBinding = {};
	RenderObjectsLayoutBinding.binding = 0;
	RenderObjectsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	RenderObjectsLayoutBinding.descriptorCount = 1;
	RenderObjectsLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	RenderObjectsLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo RenderObjectsLayoutInfo = {};
	RenderObjectsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	RenderObjectsLayoutInfo.bindingCount = 1;
	RenderObjectsLayoutInfo.pBindings = &RenderObjectsLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(m_VulkanDevice->m_Device, &RenderObjectsLayoutInfo, nullptr, &m_RenderObjectsSetLayout));

	m_ObjectsDataBuffer = vkutils::CreateBuffer(m_VulkanDevice, sizeof(sGPURenderObjectData) * MAX_RENDER_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		for (size_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vmaDestroyBuffer(m_VulkanDevice->m_Allocator, m_FramesData[i].UBOBuffer.Buffer, m_FramesData[i].UBOBuffer.Allocation);
		}

		vmaDestroyBuffer(m_VulkanDevice->m_Allocator, m_ObjectsDataBuffer.Buffer, m_ObjectsDataBuffer.Allocation);

		vkDestroyDescriptorSetLayout(m_VulkanDevice->m_Device, m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_VulkanDevice->m_Device, m_RenderObjectsSetLayout, nullptr);
	});
}

void CVulkanBackend::InitRenderPass()
{
	// Render pass creation
	VkAttachmentDescription ColorAttachment = {};
	ColorAttachment.format = m_SwapchainImageFormat;
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription DepthAttachment = {};
	DepthAttachment.format = FindDepthFormat();
	DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentRef = {};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentRef = {};
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;
	Subpass.pDepthStencilAttachment = &DepthAttachmentRef;

	VkSubpassDependency Dependency{};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> Attachments = { ColorAttachment, DepthAttachment };
	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
	RenderPassInfo.pAttachments = Attachments.data();
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &Dependency;

	VK_CHECK(vkCreateRenderPass(m_VulkanDevice->m_Device, &RenderPassInfo, nullptr, &m_RenderPass));
}

void CVulkanBackend::InitFramebuffers()
{
	// Framebuffers creation
	VkFramebufferCreateInfo FramebufferInfo = {};
	FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferInfo.pNext = nullptr;
	FramebufferInfo.renderPass = m_RenderPass;
	FramebufferInfo.attachmentCount = 1;
	FramebufferInfo.width = m_WindowExtent.width;
	FramebufferInfo.height = m_WindowExtent.height;
	FramebufferInfo.layers = 1;

	const uint32 SwapchainImageCount = m_SwapchainImages.size();
	m_Framebuffers = std::vector<VkFramebuffer>(SwapchainImageCount);

	for (int32 i = 0; i < SwapchainImageCount; ++i)
	{
		std::array<VkImageView, 2> Attachments = { m_SwapchainImageViews[i], m_DepthImageView };

		FramebufferInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
		FramebufferInfo.pAttachments = Attachments.data();
		VK_CHECK(vkCreateFramebuffer(m_VulkanDevice->m_Device, &FramebufferInfo, nullptr, &m_Framebuffers[i]));
	}

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]
	{
		for (auto Framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(m_VulkanDevice->m_Device, Framebuffer, nullptr);
		}
	});
}

void CVulkanBackend::InitPipelines()
{
	VkShaderModule VertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_VulkanDevice->m_Device, "../Engine/shaders/vert.spv", &VertShader))
	{
		std::cout << "Error when building the vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule FragShader;
	if (!vkutils::LoadShaderModule(m_VulkanDevice->m_Device, "../Engine/shaders/frag.spv", &FragShader))
	{
		std::cout << "Error when building the fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();
	std::array<VkDescriptorSetLayout, 2> SetLayouts = { m_DescriptorSetLayout, m_RenderObjectsSetLayout };
	PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(SetLayouts.size());
	PipelineLayoutInfo.pSetLayouts = SetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_VulkanDevice->m_Device, &PipelineLayoutInfo, nullptr, &m_ForwardPipelineLayout));

	PipelineBuilder PipelineBuilder;

	std::vector<VkDynamicState> DynamicStates = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	PipelineBuilder.m_DynamicState = vkinit::DynamicStateCreateInfo(DynamicStates);

	sVertexInputDescription VertexDescription = sVertex::GetVertexDescription();

	PipelineBuilder.m_VertexInputInfo = vkinit::VertexInputStateCreateInfo();
	PipelineBuilder.m_VertexInputInfo.vertexBindingDescriptionCount = VertexDescription.Bindings.size();
	PipelineBuilder.m_VertexInputInfo.pVertexBindingDescriptions = VertexDescription.Bindings.data();
	PipelineBuilder.m_VertexInputInfo.vertexAttributeDescriptionCount = VertexDescription.Attributes.size();
	PipelineBuilder.m_VertexInputInfo.pVertexAttributeDescriptions = VertexDescription.Attributes.data();

	PipelineBuilder.m_InputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	PipelineBuilder.m_Viewport.x = 0.0f;
	PipelineBuilder.m_Viewport.y = 0.0f;
	PipelineBuilder.m_Viewport.width = static_cast<float>(m_WindowExtent.width);
	PipelineBuilder.m_Viewport.height = static_cast<float>(m_WindowExtent.height);
	PipelineBuilder.m_Viewport.minDepth = 0.0f;
	PipelineBuilder.m_Viewport.maxDepth = 1.0f;

	PipelineBuilder.m_Scissor.offset = {0, 0};
	PipelineBuilder.m_Scissor.extent = m_WindowExtent;

	PipelineBuilder.m_DepthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS);
	PipelineBuilder.m_Rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	PipelineBuilder.m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	PipelineBuilder.m_Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	PipelineBuilder.m_Multisampling = vkinit::MultisamplingStateCreateInfo();
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());

	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, VertShader));
	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader));

	PipelineBuilder.m_PipelineLayout = m_ForwardPipelineLayout;

	m_ForwardPipeline = PipelineBuilder.BuildPipeline(m_VulkanDevice->m_Device, m_RenderPass);

	vkDestroyShaderModule(m_VulkanDevice->m_Device, VertShader, nullptr);
	vkDestroyShaderModule(m_VulkanDevice->m_Device, FragShader, nullptr);

	m_VulkanDevice->m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroyPipeline(m_VulkanDevice->m_Device, m_ForwardPipeline, nullptr);
		vkDestroyPipelineLayout(m_VulkanDevice->m_Device, m_ForwardPipelineLayout, nullptr);
	});
}

void CVulkanBackend::UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx)
{
	assert(ImageIdx >= 0 && ImageIdx < FRAME_OVERLAP);

	sFrameUBO FrameUBO = {};
	//FrameUBO.View = aCamera->GetView();
	glm::vec3 Position = aCamera->GetPosition();
	FrameUBO.View = glm::lookAt(Position, Position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	FrameUBO.Proj = glm::perspective(glm::radians(70.0f), m_WindowExtent.width / (float)m_WindowExtent.height, 0.1f, 200.0f);
	FrameUBO.Proj[1][1] *= -1;
	FrameUBO.ViewProj = FrameUBO.Proj * FrameUBO.View;

	memcpy(m_FramesData[ImageIdx].MappedUBOBuffer, &FrameUBO, sizeof(sFrameUBO));
}

void CVulkanBackend::RecordCommandBuffer(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx)
{
	VkCommandBufferBeginInfo BeginInfo = vkinit::CommandBufferBeginInfo();

	VK_CHECK(vkBeginCommandBuffer(aCommandBuffer, &BeginInfo));

	VkRenderPassBeginInfo RenderPassInfo = vkinit::RenderPassBeginInfo(m_RenderPass, m_WindowExtent, m_Framebuffers[aImageIdx]);

	std::array<VkClearValue, 2> ClearValues = {};
	ClearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	ClearValues[1].depthStencil = {1.0f, 0};
	
	RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
	RenderPassInfo.pClearValues = ClearValues.data();

	vkCmdBeginRenderPass(aCommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipeline);

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = static_cast<float>(m_WindowExtent.width);
	Viewport.height = static_cast<float>(m_WindowExtent.height);
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(aCommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = {0, 0};
	Scissor.extent = m_WindowExtent;
	vkCmdSetScissor(aCommandBuffer, 0, 1, &Scissor);

	const std::array<VkDescriptorSet, 2> DescriptorSets = { m_FramesData[aImageIdx].DescriptorSet, m_ObjectsDataDescriptorSet };
	vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipelineLayout, 
		0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

	VkDeviceSize Offset = 0;
	for (size_t i = 0; i < m_RenderObjectsData.size(); ++i)
	{
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &m_RenderObjectsData[i].pMesh->VertexBuffer.Buffer, &Offset);
		vkCmdBindIndexBuffer(aCommandBuffer, m_RenderObjectsData[i].pMesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
		// TODO: Batch rendering.
		vkCmdDrawIndexed(aCommandBuffer, static_cast<uint32_t>(m_RenderObjectsData[i].pMesh->Indices.size()), 1, 0, 0, i);
	}

	vkCmdEndRenderPass(aCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(aCommandBuffer));
}

void CVulkanBackend::RenderFrame(const CCamera* const aCamera)
{
	vkWaitForFences(m_VulkanDevice->m_Device, 1, &m_FramesData[m_CurrentFrame].RenderFence, VK_TRUE, UINT64_MAX);

	uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(m_VulkanDevice->m_Device, m_Swapchain, UINT64_MAX, m_FramesData[m_CurrentFrame].PresentSemaphore, VK_NULL_HANDLE, &ImageIndex);
	if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_bWasWindowResized)
	{
		m_bWasWindowResized = false;
		RecreateSwapchain();
		return;
	}
	else if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	UpdateFrameUBO(aCamera, m_CurrentFrame);

	// Delay fence reset to prevent possible deadlock when recreating the swapchain.
	vkResetFences(m_VulkanDevice->m_Device, 1, &m_FramesData[m_CurrentFrame].RenderFence);

	vkResetCommandBuffer(m_FramesData[m_CurrentFrame].MainCommandBuffer, 0);
	RecordCommandBuffer(m_FramesData[m_CurrentFrame].MainCommandBuffer, ImageIndex);

	VkSubmitInfo SubmitInfo = vkinit::SubmitInfo(&m_FramesData[m_CurrentFrame].MainCommandBuffer);

	VkSemaphore WaitSemaphores[] = {m_FramesData[m_CurrentFrame].PresentSemaphore};
	VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;

	VkSemaphore SignalSemaphores[] = {m_FramesData[m_CurrentFrame].RenderSemaphore};
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	VK_CHECK(vkQueueSubmit(m_VulkanDevice->m_GraphicsQueue, 1, &SubmitInfo, m_FramesData[m_CurrentFrame].RenderFence));

	VkPresentInfoKHR PresentInfo = vkinit::PresentInfo();
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	VkSwapchainKHR SwapChains[] = {m_Swapchain};
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = SwapChains;
	PresentInfo.pImageIndices = &ImageIndex;

	vkQueuePresentKHR(m_VulkanDevice->m_GraphicsQueue, &PresentInfo);

	m_CurrentFrame = (m_CurrentFrame + 1) % FRAME_OVERLAP;
}

void CVulkanBackend::RecreateSwapchain()
{
	SGSDEBUG("Recreating swapchain...");

	vkDeviceWaitIdle(m_VulkanDevice->m_Device);

	CleanupSwapchain();

	InitSwapchain();
	InitFramebuffers();
}

 void CVulkanBackend::CleanupSwapchain()
 {
	for (size_t i = 0; i < m_Framebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(m_VulkanDevice->m_Device, m_Framebuffers[i], nullptr);
	}
	
	for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(m_VulkanDevice->m_Device, m_SwapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_VulkanDevice->m_Device, m_Swapchain, nullptr);
 }

VkFormat CVulkanBackend::FindSupportedFormat (const std::vector<VkFormat>& aCandidates, VkImageTiling aTiling, VkFormatFeatureFlags aFeatures)
{
	for (VkFormat Format : aCandidates)
	{
		VkFormatProperties Properties;
		vkGetPhysicalDeviceFormatProperties(m_VulkanDevice->m_PhysicalDevice, Format, &Properties);

		if (aTiling == VK_IMAGE_TILING_LINEAR && (Properties.linearTilingFeatures & aFeatures) == aFeatures)
		{
			return Format;
		}
		else if (aTiling == VK_IMAGE_TILING_OPTIMAL && (Properties.optimalTilingFeatures & aFeatures) == aFeatures)
		{
			return Format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}

VkFormat CVulkanBackend::FindDepthFormat()
{
	return FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool CVulkanBackend::HasStencilComponent(VkFormat aFormat)
{
	return aFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || aFormat == VK_FORMAT_D24_UNORM_S8_UINT;
}
