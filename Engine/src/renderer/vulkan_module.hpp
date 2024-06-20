#pragma once

#include "vk_types.hpp"

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

struct sUploadContext
{
    VkFence m_UploadFence;
    VkCommandPool m_CommandPool;
};

class VulkanModule
{
public:
    VulkanModule();

    bool Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);

    void Render();

    bool Shutdown();

    void HandleWindowResize();

    void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& aFunction);

private:
    void InitEnabledFeatures();

    void InitVulkan(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);
    void InitSwapchain();
    void InitCommandPools();
    void InitSyncStructures();
    void InitDescriptorSetPool();
    void InitDescriptorSetLayouts();
    void InitRenderPass();
    void InitFramebuffers();
    void InitPipelines();

    void RecordCommandBuffer(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);
    void RenderFrame();

    void RecreateSwapchain();
    // TODO: This will destroy resources that are also destroyed in the main deletion queue. Do not store resources to be deleted in a queue. Delete them manually.
    void CleanupSwapchain();

    bool m_bIsInitialized;

    // Vulkan core.
    VkInstance m_VulkanInstance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VmaAllocator m_Allocator;
    // ------------

    // pNext features.
	VkPhysicalDeviceBufferDeviceAddressFeatures m_EnabledBufferDeviceAddressFeatures{};
    void* m_pDeviceCreatepNextChain = nullptr;
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

    bool m_bWasWindowResized;

    sDeletionQueue m_MainDeletionQueue;

    sUploadContext m_UploadContext;

    const std::vector<sVertex> m_Vertices =  
    {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    AllocatedBuffer m_VertexBuffer;
};