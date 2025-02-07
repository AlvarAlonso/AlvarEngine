#include <renderer/vulkan/vulkan_backend.hpp>
#include <renderer/vulkan/vulkan_device.hpp>
#include <renderer/vulkan/vulkan_swapchain.hpp>
#include <engine.hpp>
#include <core/logger.h>

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
namespace Alvar
{
    void CVulkanBackend::InitImGuiBackend()
    {
        SGSINFO("Init ImGui Vulkan Backend");
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(CEngine::Get()->GetWindow(), true);
        
        ImGui_ImplVulkan_InitInfo InitInfo = {};
        InitInfo.Instance = m_pVulkanDevice->m_VulkanInstance;
        InitInfo.PhysicalDevice = m_pVulkanDevice->m_PhysicalDevice;
        InitInfo.Device = m_pVulkanDevice->m_Device;
        InitInfo.QueueFamily = m_pVulkanDevice->m_GraphicsQueueFamily;
        InitInfo.Queue = m_pVulkanDevice->m_GraphicsQueue;
        InitInfo.PipelineCache = VK_NULL_HANDLE;
        InitInfo.DescriptorPool = m_DescriptorPool;
        InitInfo.RenderPass = m_pVulkanSwapchain->m_RenderPass;
        InitInfo.Subpass = 0;
        InitInfo.MinImageCount = 2;
        InitInfo.ImageCount = 2;
        InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        InitInfo.Allocator = nullptr;
        InitInfo.CheckVkResultFn = VK_NULL_HANDLE;
        
        ImGui_ImplVulkan_Init(&InitInfo);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void CVulkanBackend::ShutdownImGuiBackend()
    {
        ImGui_ImplVulkan_Shutdown();
    }

	void CVulkanBackend::OnBeforeRenderEnd(VkCommandBuffer aCommandBuffer)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), aCommandBuffer);
	}

    void CVulkanBackend::ImGuiBeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
    }

    void CVulkanBackend::ImGuiEndFrame()
    {
        ImGui::EndFrame();
    }

    uint32_t CVulkanBackend::GetImGuiVulkanPoolSize()
    {
        return IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    }
}
