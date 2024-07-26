#pragma once

#include "vk_types.hpp"
#include <core/types.hpp>

struct sUploadContext
{
    VkFence m_UploadFence;
    VkCommandPool m_CommandPool;
};

class CVulkanDevice
{
public:
    CVulkanDevice();
    ~CVulkanDevice();

    void InitVulkanDevice();

    void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& aFunction) const;
    
    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& aCandidates, VkImageTiling aTiling, VkFormatFeatureFlags aFeatures);
    
    // Vulkan Core.
    VkInstance m_VulkanInstance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VmaAllocator m_Allocator;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    sDeletionQueue m_MainDeletionQueue;
    VkQueue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamily;
    sUploadContext m_UploadContext;

private:
    void InitEnabledFeatures();

    // pNext features.
	VkPhysicalDeviceBufferDeviceAddressFeatures m_EnabledBufferDeviceAddressFeatures{};
    void* m_pDeviceCreatepNextChain = nullptr;
    // ------------
};

CVulkanDevice* GetVulkanDevice();