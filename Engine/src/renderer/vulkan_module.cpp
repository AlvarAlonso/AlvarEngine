#include "vulkan_module.hpp"

#include "core/defines.h"
#include "core/logger.h"
#include "vk_types.hpp"
#include "vk_initializers.hpp"

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

		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

		for (int32 i = 0; i < m_SwapchainImageViews.size(); ++i)
		{
			vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
		}

		vkDestroyDevice(m_Device, nullptr);
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
		vkb::destroy_debug_utils_messenger(m_VulkanInstance, m_DebugMessenger);

		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

    return false;
}
