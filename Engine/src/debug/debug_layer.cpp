#pragma once

#include "debug_layer.hpp"
#include <core/logger.h>
#include <engine.hpp>
#include <renderer/render_module.hpp>

#include <imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

// TODO: Abstract the render API from here. There shouldn't be any Vulkan mentions in here.

namespace Alvar
{
    CDebugLayer::CDebugLayer() :
        ILayer("Debug Layer")
    {

    }

    void CDebugLayer::OnAttach()
    {
        SGSINFO("On Attach: %s.", m_DebugName.c_str());
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(CEngine::Get()->GetWindow(), false);
        
        ImGui_ImplVulkan_InitInfo VulkanInitInfo = {};
        // TODO: Module manager.
        CEngine::Get()->GetRenderModule()->PopulateImGuiDeviceInitInfo(&VulkanInitInfo);
        ImGui_ImplVulkan_Init(&VulkanInitInfo);
        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void CDebugLayer::OnDetach()
    {
        SGSINFO("On Detach: %s.", m_DebugName.c_str());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void CDebugLayer::OnEvent(CEvent& aEvent)
    {

    }

    void CDebugLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
    }
    
    void CDebugLayer::End()
    {
        //ImGui::Render();
        //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),);
    }
}