#include "vulkan_swapchain.hpp"
#include "vulkan_device.hpp"
#include "vk_initializers.hpp"
#include "engine.hpp"
#include "core/logger.h"

#include <VulkanBootstrap/VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <array>

CVulkanSwapchain::CVulkanSwapchain(CVulkanDevice* aVulkanDevice)
{
    m_VulkanDevice = aVulkanDevice;
}

CVulkanSwapchain::~CVulkanSwapchain()
{
    vkDestroyRenderPass(m_VulkanDevice->m_Device, m_RenderPass, nullptr);

    CleanupSwapchain();
}

void CVulkanSwapchain::InitVulkanSwapchain()
{
    InitSwapchain();
    InitRenderPass();
    InitDepthBuffer();
    InitFramebuffers();
}

void CVulkanSwapchain::InitSwapchain()
{
	int Width = 0;
	int Height = 0;
	glfwGetFramebufferSize(CEngine::Get()->GetWindow(), &Width, &Height);

	m_WindowExtent.width = static_cast<uint32_t>(Width);
	m_WindowExtent.height = static_cast<uint32_t>(Height);

	SGSINFO("Creating Swapchain. Framebuffer Size: %d, %d.", m_WindowExtent.width, m_WindowExtent.height);

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

void CVulkanSwapchain::InitFramebuffers()
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

void CVulkanSwapchain::InitRenderPass()
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
	DepthAttachment.format = m_VulkanDevice->FindDepthFormat();
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

void CVulkanSwapchain::InitDepthBuffer()
{
    VkFormat DepthFormat = m_VulkanDevice->FindDepthFormat();

	VkImageCreateInfo DepthImgInfo = vkinit::ImageCreateInfo(DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, {m_WindowExtent.width, m_WindowExtent.height, 1});

	VmaAllocationCreateInfo DepthImgAllocInfo = {};
	DepthImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	DepthImgAllocInfo.requiredFlags = VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_VulkanDevice->m_Allocator, &DepthImgInfo, &DepthImgAllocInfo, &m_DepthImage.Image, &m_DepthImage.Allocation, nullptr);

	VkImageViewCreateInfo DepthViewInfo = vkinit::ImageViewCreateInfo(DepthFormat, m_DepthImage.Image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(m_VulkanDevice->m_Device, &DepthViewInfo, nullptr, &m_DepthImageView));
}

void CVulkanSwapchain::RecreateSwapchain()
{
	SGSDEBUG("Recreating swapchain...");
    
	int Width = 0;
	int Height = 0;
	glfwGetFramebufferSize(CEngine::Get()->GetWindow(), &Width, &Height);
	while (Width == 0 || Height == 0) 
	{
		glfwGetFramebufferSize(CEngine::Get()->GetWindow(), &Width, &Height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_VulkanDevice->m_Device);

    SGSDEBUG("MANUELA");

	CleanupSwapchain();
    
	InitSwapchain();
    InitDepthBuffer();

	InitFramebuffers();
}

void CVulkanSwapchain::CleanupSwapchain()
{
    vmaDestroyImage(m_VulkanDevice->m_Allocator, m_DepthImage.Image, m_DepthImage.Allocation);

    for (size_t i = 0; i < m_Framebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(m_VulkanDevice->m_Device, m_Framebuffers[i], nullptr);
        m_Framebuffers.clear();
	}

	for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(m_VulkanDevice->m_Device, m_SwapchainImageViews[i], nullptr);
        m_SwapchainImageViews.clear();
	}

	vkDestroySwapchainKHR(m_VulkanDevice->m_Device, m_Swapchain, nullptr);
}
