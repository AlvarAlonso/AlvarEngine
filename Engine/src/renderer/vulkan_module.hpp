#pragma once

#include "vk_types.hpp"

#include <vector>
#include <deque>
#include <functional>

constexpr uint32_t FRAME_OVERLAP = 3;

struct sDeletionQueue
{
	std::deque<std::function<void()>> Deletors;

	void PushFunction(std::function<void()>&& function)
	{
		Deletors.push_back(function);
	}

	void Flush()
	{
		for (auto it = Deletors.rbegin(); it != Deletors.rend(); it++)
		{
			(*it)();
		}

		Deletors.clear();
	}
};

struct sFrameData
{
    VkSemaphore PresentSemaphore;
    VkSemaphore RenderSemaphore;
    VkFence RenderFence;
    VkCommandPool CommandPool;
    VkCommandBuffer MainCommandBuffer;
};

class VulkanModule
{
public:
    VulkanModule();

    bool Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);

    void Render();

    bool Shutdown();

private:
    void InitVulkan(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);
    void InitCommandPools();
    void InitSyncStructures();
    void InitDescriptorSetPool();
    void InitDescriptorSetLayouts();
    void InitRenderPass();
    void InitFramebuffers();
    void InitPipelines();

    void RecordCommandBuffer(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);
    void RenderFrame();

    bool m_bIsInitialized;

    // Vulkan core.
    VkInstance m_VulkanInstance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    // ------------

    // Swapchain.
    VkSwapchainKHR m_Swapchain;
    VkFormat m_SwapchainImageFormat;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    uint32_t m_ImageIdx;
    // ------------

    // Vulkan Commands.
    VkQueue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamily;

    VkCommandPool m_CommandPool;
    // ------------

    // Render passes.
    VkRenderPass m_RenderPass;
    std::vector<VkFramebuffer> m_Framebuffers;
    // ------------

    // Pipelines.
    VkPipelineLayout m_ForwardPipelineLayout;
    VkPipeline m_ForwardPipeline;
    //-------------

    sFrameData m_FramesData[FRAME_OVERLAP];
    uint32_t m_CurrentFrame;

    VkExtent2D m_WindowExtent = { 800, 600 };

    sDeletionQueue m_MainDeletionQueue;
};