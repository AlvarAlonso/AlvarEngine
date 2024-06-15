#include "vulkan_module.hpp"

#include "core/defines.h"
#include "core/logger.h"
#include "vk_types.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"

// TODO: Include path correctly
#include "../../ThirdParty/VulkanBootstrap/VkBootstrap.h"

#include <iostream>

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			std::abort();                                                \
		}                                                           \
	} while (0)														\

VulkanModule::VulkanModule()
{

}

bool VulkanModule::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
	InitVulkan(aInstanceHandle, aWindowHandle);

	InitCommandPools();

	InitRenderPass();

	InitFramebuffers();

    m_bIsInitialized = true;

    return true;
}

void VulkanModule::Render()
{

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

	// TODO: use VMA
	VK_CHECK(vkCreateWin32SurfaceKHR(m_VulkanInstance, &SurfaceCreateInfo, nullptr, &m_Surface));

	// Select physical device.
	vkb::PhysicalDeviceSelector PhysicalDeviceSelector {vkbInstance};
	vkb::PhysicalDevice vkbPhysicalDevice = PhysicalDeviceSelector
	.set_minimum_version(1, 2)
	.set_surface(m_Surface)
	.select()
	.value();

	// Create logical device.
	vkb::DeviceBuilder DeviceBuilder{vkbPhysicalDevice};
	vkb::Device vkbDevice = DeviceBuilder.build().value();

	// initialize device
	m_PhysicalDevice = vkbPhysicalDevice.physical_device;
	m_Device = vkbDevice.device;

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

	// Get Graphics Queue
	m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

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

	VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_CommandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_Device, &CmdAllocInfo, &m_CommandBuffer));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
	});
}

void VulkanModule::InitSyncStructures()
{

}

void VulkanModule::InitDescriptorSetPool()
{

}

void VulkanModule::InitDescriptorSetLayouts()
{

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

	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = 1;
	RenderPassInfo.pAttachments = &ColorAttachment;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;

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
}

void VulkanModule::InitPipelines()
{
	VkShaderModule VertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_Device, "../shaders/vert.spv", &VertShader))
	{
		std::cout << "Error when building the vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule FragShader;
	if (!vkutils::LoadShaderModule(m_Device, "../shaders/frag.spv", &FragShader))
	{
		std::cout << "Error when building the fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();

	VK_CHECK(vkCreatePipelineLayout(m_Device, &PipelineLayoutInfo, nullptr, &m_ForwardPipelineLayout));

	PipelineBuilder PipelineBuilder;

	PipelineBuilder.m_VertexInputInfo = vkinit::VertexInputStateCreateInfo();

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
