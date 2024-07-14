#pragma once

#include "vk_types.hpp"

class CVulkanDevice;

class CVulkanSwapchain
{
public:
    CVulkanSwapchain(CVulkanDevice* aVulkanDevice);
    ~CVulkanSwapchain();

    void InitVulkanSwapchain();
    void RecreateSwapchain();

private:
    void InitSwapchain();
    // TODO: Can we move it to vulkan backend? Swapchain probably should not have info about the render passes and its attachments.
    void InitRenderPass();
    void InitDepthBuffer();
    void InitFramebuffers();

    // TODO: This will destroy resources that are also destroyed in the main deletion queue. Do not store resources to be deleted in a queue. Delete them manually.
    void CleanupSwapchain();
    
    CVulkanDevice* m_VulkanDevice;

public:
    // Swapchain.
    VkSwapchainKHR m_Swapchain;
    VkFormat m_SwapchainImageFormat;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    uint32_t m_ImageIdx;
    // ------------

    // Render passes.
    VkRenderPass m_RenderPass;
    std::vector<VkFramebuffer> m_Framebuffers;
    // ------------

    AllocatedImage m_DepthImage;
    VkImageView m_DepthImageView;

    VkExtent2D m_WindowExtent = { 800, 600 };
};