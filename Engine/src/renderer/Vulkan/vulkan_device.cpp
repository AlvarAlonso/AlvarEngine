#include "vulkan_device.hpp"
#include "vk_initializers.hpp"
#include <engine.hpp>

#include <VulkanBootstrap/VkBootstrap.h>
#include <GLFW/glfw3.h>

#include "core/logger.h"
#include <iostream>

CVulkanDevice::CVulkanDevice()
{

}

CVulkanDevice::~CVulkanDevice()
{
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
    vkb::destroy_debug_utils_messenger(m_VulkanInstance, m_DebugMessenger);

    vkDestroyInstance(m_VulkanInstance, nullptr);
}

void CVulkanDevice::InitVulkanDevice()
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

	if (glfwCreateWindowSurface(m_VulkanInstance, CEngine::Get()->GetWindow(), nullptr, &m_Surface))
	{
		throw std::runtime_error("Failed to create window surface!");
	}
	
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
}

void CVulkanDevice::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& aFunction) const
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

VkFormat CVulkanDevice::FindDepthFormat()
{
    return FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat CVulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& aCandidates, VkImageTiling aTiling, VkFormatFeatureFlags aFeatures)
{
	for (VkFormat Format : aCandidates)
	{
		VkFormatProperties Properties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, Format, &Properties);

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

void CVulkanDevice::InitEnabledFeatures()
{
	m_EnabledBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	m_EnabledBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	m_EnabledBufferDeviceAddressFeatures.pNext = nullptr;

	m_pDeviceCreatepNextChain = &m_EnabledBufferDeviceAddressFeatures;
}