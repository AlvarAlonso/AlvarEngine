#pragma once

#include "vk_types.hpp"

#include <deque>
#include <functional>

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

    void InitVulkanDevice(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);

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