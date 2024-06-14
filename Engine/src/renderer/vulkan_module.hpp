#pragma once

#include "vk_types.hpp"

#include <vector>

class VulkanModule
{
public:
    VulkanModule();

    bool Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);

    void Render();

    bool Shutdown();

private:
    bool m_bIsInitialized = false;

    // Vulkan core

    VkInstance m_VulkanInstance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;

    // ------------

    // Swapchain

    VkSwapchainKHR m_Swapchain;
    VkFormat m_SwapchainImageFormat;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;

    // ------------

    // Vulkan Commands

    VkQueue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamily;

    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;

    // ------------

    // Render passes

    VkRenderPass m_RenderPass;
    std::vector<VkFramebuffer> m_Framebuffers;

    // ------------

    VkExtent2D m_WindowExtent = { 800, 600 };
};