#include "vulkan_module.hpp"

#define GLM_FORCE_RADIANS

#include "engine.hpp"
#include "core/defines.h"
#include "core/logger.h"
#include "vk_types.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"

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

VulkanModule::VulkanModule() :
	m_bIsInitialized(false),
	m_ImageIdx(0),
	m_CurrentFrame(0),
	m_bWasWindowResized(false)
{
}

bool VulkanModule::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
	InitVulkan(aInstanceHandle, aWindowHandle);

	InitSwapchain();

	InitCommandPools();

	InitRenderPass();

	InitFramebuffers();

	InitDescriptorSetLayouts();

	InitDescriptorSetPool();

	InitPipelines();

	InitSyncStructures();

	InitTextureSamplers();

	vkutils::CreateVertexBuffer(m_Allocator, m_Vertices, m_VertexBuffer);
	vkutils::CreateIndexBuffer(m_Allocator, m_Indices, m_IndexBuffer);

	vkutils::LoadImageFromFile(m_Allocator, "../Resources/Images/DonPatch.jpg", m_Image);
	
	// TODO: Placeholder for testing purposes. TO BE REMOVED.
	VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(m_Device, &ViewInfo, nullptr, &m_ImageView));

	InitDescriptorSets();

    m_bIsInitialized = true;

    return true;
}

void VulkanModule::Render()
{
	assert(m_bIsInitialized);

	RenderFrame();
}

bool VulkanModule::Shutdown()
{
	if (m_bIsInitialized)
	{
		SGSINFO("Shutting down Vulkan");

		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		for (int32 i = 0; i < m_SwapchainImageViews.size(); ++i)
		{
			vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
			vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
		}

		vkDestroyDevice(m_Device, nullptr);
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		vkb::destroy_debug_utils_messenger(m_VulkanInstance, m_DebugMessenger);

		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

    return false;
}

void VulkanModule::HandleWindowResize()
{
	m_bWasWindowResized = true;
}

void VulkanModule::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& aFunction)
{
	SGSDEBUG("Immediate Submit");

	VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_UploadContext.m_CommandPool, 1);

	VkCommandBuffer Cmd;
	VK_CHECK(vkAllocateCommandBuffers(m_Device, &CmdAllocInfo, &Cmd));

	VkCommandBufferBeginInfo CmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		
	VK_CHECK(vkBeginCommandBuffer(Cmd, &CmdBeginInfo));

	aFunction(Cmd);

	VK_CHECK(vkEndCommandBuffer(Cmd));

	VkSubmitInfo Submit = vkinit::SubmitInfo(&Cmd);

	VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &Submit, m_UploadContext.m_UploadFence));

	vkWaitForFences(m_Device, 1, &m_UploadContext.m_UploadFence, true, UINT64_MAX);
	vkResetFences(m_Device, 1, &m_UploadContext.m_UploadFence);

	vkResetCommandPool(m_Device, m_UploadContext.m_CommandPool, 0);
}

void VulkanModule::InitEnabledFeatures()
{
	m_EnabledBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	m_EnabledBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	m_EnabledBufferDeviceAddressFeatures.pNext = nullptr;

	m_pDeviceCreatepNextChain = &m_EnabledBufferDeviceAddressFeatures;
}

void VulkanModule::InitVulkan(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
	vkb::InstanceBuilder InstanceBuilder;

	auto VulkanInstanceResult = InstanceBuilder.set_app_name("AlvarEngine")
	.request_validation_layers(true)
	.require_api_version(1, 2, 0)
	.use_default_debug_messenger()
	.build();

	vkb::Instance vkbInstance = VulkanInstanceResult.value();

	m_VulkanInstance = vkbInstance.instance;

	m_DebugMessenger = vkbInstance.debug_messenger;

	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo{};
	SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceCreateInfo.hwnd = aWindowHandle;
	SurfaceCreateInfo.hinstance = aInstanceHandle;

	VK_CHECK(vkCreateWin32SurfaceKHR(m_VulkanInstance, &SurfaceCreateInfo, nullptr, &m_Surface));
	
	VkPhysicalDeviceFeatures RequiredFeatures;
	RequiredFeatures.samplerAnisotropy = VK_TRUE;
	
	// Select physical device.
	vkb::PhysicalDeviceSelector PhysicalDeviceSelector {vkbInstance};
	vkb::PhysicalDevice vkbPhysicalDevice = PhysicalDeviceSelector
	.set_minimum_version(1, 2)
	.set_surface(m_Surface)
	//.set_required_features(RequiredFeatures)
	.select()
	.value();

	InitEnabledFeatures();

	// Create logical device.
	vkb::DeviceBuilder DeviceBuilder{vkbPhysicalDevice};
	vkb::Device vkbDevice = DeviceBuilder.add_pNext(m_pDeviceCreatepNextChain).build().value();

	// initialize device
	m_PhysicalDevice = vkbPhysicalDevice.physical_device;
	m_Device = vkbDevice.device;

	// Get Graphics Queue
	m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo AllocatorInfo = {};
	AllocatorInfo.physicalDevice = m_PhysicalDevice;
	AllocatorInfo.device = m_Device;
	AllocatorInfo.instance = m_VulkanInstance;
	AllocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&AllocatorInfo, &m_Allocator);

	m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	});
}

void VulkanModule::InitSwapchain()
{
	sDimension2D WindowDimensions = Engine::Get()->GetAppWindowDimensions();
	m_WindowExtent = {WindowDimensions.Width, WindowDimensions.Height};

	SGSINFO("Creating Swapchain. Framebuffer Size: %d, %d.", WindowDimensions.Width, WindowDimensions.Height);

	// TODO: initialize extent according to Window's created window
	// Swapchain initialization.
	vkb::SwapchainBuilder SwapchainBuilder{m_PhysicalDevice, m_Device, m_Surface};
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

	m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	});
}

void VulkanModule::InitCommandPools()
{
	// Creation of command structures
	VkCommandPoolCreateInfo CommandPoolInfo = vkinit::CommandPoolCreateInfo(m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(m_Device, &CommandPoolInfo, nullptr, &m_CommandPool));

	for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_CommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(m_Device, &CmdAllocInfo, &m_FramesData[i].MainCommandBuffer));
	}

	VkCommandPoolCreateInfo UploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(m_GraphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(m_Device, &UploadCommandPoolInfo, nullptr, &m_UploadContext.m_CommandPool));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_UploadContext.m_CommandPool, nullptr);
	});
}

void VulkanModule::InitSyncStructures()
{
	VkSemaphoreCreateInfo SemaphoreInfo = vkinit::SemaphoreCreateInfo();

	for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VK_CHECK(vkCreateSemaphore(m_Device, &SemaphoreInfo, nullptr, &m_FramesData[i].PresentSemaphore));
		VK_CHECK(vkCreateSemaphore(m_Device, &SemaphoreInfo, nullptr, &m_FramesData[i].RenderSemaphore));

		VkFenceCreateInfo FenceInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		VK_CHECK(vkCreateFence(m_Device, &FenceInfo, nullptr, &m_FramesData[i].RenderFence));
	}

	VkFenceCreateInfo UploadFenceInfo = vkinit::FenceCreateInfo();
	VK_CHECK(vkCreateFence(m_Device, &UploadFenceInfo, nullptr, &m_UploadContext.m_UploadFence));

	m_MainDeletionQueue.PushFunction([=]
	{
		for (int32_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vkDestroySemaphore(m_Device, m_FramesData[i].PresentSemaphore, nullptr);
			vkDestroySemaphore(m_Device, m_FramesData[i].RenderSemaphore, nullptr);
			vkDestroyFence(m_Device, m_FramesData[i].RenderFence, nullptr);
			vkDestroyFence(m_Device, m_UploadContext.m_UploadFence, nullptr);
		}
	});
}

void VulkanModule::InitTextureSamplers()
{
	VkPhysicalDeviceProperties Properties = {};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &Properties);

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

	VK_CHECK(vkCreateSampler(m_Device, &SamplerInfo, nullptr, &m_DefaultSampler));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroySampler(m_Device, m_DefaultSampler, nullptr);
	});
}

void VulkanModule::InitDescriptorSetPool()
{
	VkDescriptorPoolSize UBOPoolSize = {};
	UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOPoolSize.descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

	VkDescriptorPoolSize SamplerPoolSize = {};
	SamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerPoolSize.descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

	std::array<VkDescriptorPoolSize, 2> PoolSizes = { UBOPoolSize, SamplerPoolSize };

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
	PoolInfo.pPoolSizes = PoolSizes.data();;
	PoolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP);

	VK_CHECK(vkCreateDescriptorPool(m_Device, &PoolInfo, nullptr, &m_DescriptorPool));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	});
}

void VulkanModule::InitDescriptorSets()
{
	VkDescriptorSetAllocateInfo AllocInfo{};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.descriptorPool = m_DescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &m_DescriptorSetLayout;

	for (size_t i = 0; i < FRAME_OVERLAP; ++i)
	{
		VK_CHECK(vkAllocateDescriptorSets(m_Device, &AllocInfo, &m_FramesData[i].DescriptorSet));
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

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}
}

void VulkanModule::InitDescriptorSetLayouts()
{
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

	VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &LayoutInfo, nullptr, &m_DescriptorSetLayout));

	VkDeviceSize BufferSize = sizeof(sFrameUBO);
	for (int i = 0; i < FRAME_OVERLAP; ++i)
	{
		m_FramesData[i].UBOBuffer = vkutils::CreateBuffer(m_Allocator, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		vmaMapMemory(m_Allocator, m_FramesData[i].UBOBuffer.Allocation, &m_FramesData[i].MappedUBOBuffer);
	}

	m_MainDeletionQueue.PushFunction([=]
	{
		for (size_t i = 0; i < FRAME_OVERLAP; ++i)
		{
			vmaDestroyBuffer(m_Allocator, m_FramesData[i].UBOBuffer.Buffer, m_FramesData[i].UBOBuffer.Allocation);
		}

		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
	});
}

void VulkanModule::InitRenderPass()
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

	VkAttachmentReference ColorAttachmentRef = {};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;

	VkSubpassDependency Dependency{};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = 1;
	RenderPassInfo.pAttachments = &ColorAttachment;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &Dependency;

	VK_CHECK(vkCreateRenderPass(m_Device, &RenderPassInfo, nullptr, &m_RenderPass));
}

void VulkanModule::InitFramebuffers()
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
		FramebufferInfo.pAttachments = &m_SwapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(m_Device, &FramebufferInfo, nullptr, &m_Framebuffers[i]));
	}

	m_MainDeletionQueue.PushFunction([=]
	{
		for (auto Framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(m_Device, Framebuffer, nullptr);
		}
	});
}

void VulkanModule::InitPipelines()
{
	VkShaderModule VertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_Device, "../Engine/shaders/vert.spv", &VertShader))
	{
		std::cout << "Error when building the vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule FragShader;
	if (!vkutils::LoadShaderModule(m_Device, "../Engine/shaders/frag.spv", &FragShader))
	{
		std::cout << "Error when building the fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();
	PipelineLayoutInfo.setLayoutCount = 1;
	PipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

	VK_CHECK(vkCreatePipelineLayout(m_Device, &PipelineLayoutInfo, nullptr, &m_ForwardPipelineLayout));

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

	PipelineBuilder.m_DepthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
	PipelineBuilder.m_Rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	PipelineBuilder.m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	PipelineBuilder.m_Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	PipelineBuilder.m_Multisampling = vkinit::MultisamplingStateCreateInfo();
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());

	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, VertShader));
	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader));

	PipelineBuilder.m_PipelineLayout = m_ForwardPipelineLayout;

	m_ForwardPipeline = PipelineBuilder.BuildPipeline(m_Device, m_RenderPass);

	vkDestroyShaderModule(m_Device, VertShader, nullptr);
	vkDestroyShaderModule(m_Device, FragShader, nullptr);

	m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroyPipeline(m_Device, m_ForwardPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_ForwardPipelineLayout, nullptr);
	});
}

void VulkanModule::UpdateFrameUBO(uint32_t ImageIdx)
{
	assert(ImageIdx >= 0 && ImageIdx < FRAME_OVERLAP);

	static auto StartTime = std::chrono::high_resolution_clock::now();

	const auto CurrentTime = std::chrono::high_resolution_clock::now();
	const float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();

	sFrameUBO FrameUBO = {};
	FrameUBO.Model = glm::rotate(glm::mat4(1.0f), Time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	FrameUBO.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	FrameUBO.Proj = glm::perspective(glm::radians(45.0f), m_WindowExtent.width / (float) m_WindowExtent.height, 0.1f, 10.0f);
	FrameUBO.Proj[1][1] *= -1;


	memcpy(m_FramesData[ImageIdx].MappedUBOBuffer, &FrameUBO, sizeof(sFrameUBO));
}

void VulkanModule::RecordCommandBuffer(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx)
{
	VkCommandBufferBeginInfo BeginInfo = vkinit::CommandBufferBeginInfo();

	VK_CHECK(vkBeginCommandBuffer(aCommandBuffer, &BeginInfo));

	VkRenderPassBeginInfo RenderPassInfo = vkinit::RenderPassBeginInfo(m_RenderPass, m_WindowExtent, m_Framebuffers[aImageIdx]);

	VkClearValue ClearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	RenderPassInfo.clearValueCount = 1;
	RenderPassInfo.pClearValues = &ClearColor;

	vkCmdBeginRenderPass(aCommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipeline);
	VkDeviceSize Offset = 0;
	vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &m_VertexBuffer.Buffer, &Offset);
	vkCmdBindIndexBuffer(aCommandBuffer, m_IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

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

	vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipelineLayout, 0, 1, &m_FramesData[aImageIdx].DescriptorSet, 0, nullptr);
	vkCmdDrawIndexed(aCommandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(aCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(aCommandBuffer));
}

void VulkanModule::RenderFrame()
{
	vkWaitForFences(m_Device, 1, &m_FramesData[m_CurrentFrame].RenderFence, VK_TRUE, UINT64_MAX);

	uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_FramesData[m_CurrentFrame].PresentSemaphore, VK_NULL_HANDLE, &ImageIndex);
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

	UpdateFrameUBO(m_CurrentFrame);

	// Delay fence reset to prevent possible deadlock when recreating the swapchain.
	vkResetFences(m_Device, 1, &m_FramesData[m_CurrentFrame].RenderFence);

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

	VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &SubmitInfo, m_FramesData[m_CurrentFrame].RenderFence));

	VkPresentInfoKHR PresentInfo = vkinit::PresentInfo();
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	VkSwapchainKHR SwapChains[] = {m_Swapchain};
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = SwapChains;
	PresentInfo.pImageIndices = &ImageIndex;

	vkQueuePresentKHR(m_GraphicsQueue, &PresentInfo);

	m_CurrentFrame = (m_CurrentFrame + 1) % FRAME_OVERLAP;
}

void VulkanModule::RecreateSwapchain()
{
	SGSDEBUG("Recreating swapchain...");

	vkDeviceWaitIdle(m_Device);

	CleanupSwapchain();

	InitSwapchain();
	InitFramebuffers();
}

 void VulkanModule::CleanupSwapchain()
 {
	for (size_t i = 0; i < m_Framebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
	}
	
	for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
 }
