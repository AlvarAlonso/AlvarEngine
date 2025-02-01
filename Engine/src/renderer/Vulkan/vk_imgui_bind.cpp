#include <renderer/vulkan/vulkan_backend.hpp>
#include <renderer/vulkan/vulkan_device.hpp>
#include <renderer/vulkan/vulkan_swapchain.hpp>
#include <imgui/backends/imgui_impl_vulkan.h>

namespace Alvar
{
    void CVulkanBackend::PopulateImGuiVulkanInitInfo(void* aInitInfo)
    {
        ImGui_ImplVulkan_InitInfo* InitInfo = static_cast<ImGui_ImplVulkan_InitInfo*>(aInitInfo);
        if (InitInfo)
        {
            InitInfo->Instance = m_pVulkanDevice->m_VulkanInstance;
            InitInfo->PhysicalDevice = m_pVulkanDevice->m_PhysicalDevice;
            InitInfo->Device = m_pVulkanDevice->m_Device;
            InitInfo->QueueFamily = m_pVulkanDevice->m_GraphicsQueueFamily;
            InitInfo->Queue = m_pVulkanDevice->m_GraphicsQueue;
            InitInfo->PipelineCache = VK_NULL_HANDLE;
            InitInfo->DescriptorPool = m_DescriptorPool;
            InitInfo->RenderPass = m_pVulkanSwapchain->m_RenderPass;
            InitInfo->Subpass = 0;
            InitInfo->MinImageCount = 2;
            InitInfo->ImageCount = 2;
            InitInfo->MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            InitInfo->Allocator = nullptr;
            InitInfo->CheckVkResultFn = VK_NULL_HANDLE;
        }
    }
}
