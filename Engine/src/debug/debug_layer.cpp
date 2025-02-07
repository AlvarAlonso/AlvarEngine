#pragma once

#include "debug_layer.hpp"
#include <core/logger.h>
#include <engine.hpp>
#include <renderer/render_module.hpp>
#include <core/event_system/event_base.hpp>

#include <imgui.h>

// TODO: Abstract the render API from here. There shouldn't be any Vulkan mentions in here.

namespace Alvar
{
    CDebugLayer::CDebugLayer() :
        ILayer("Debug Layer"), m_BlockEvents(false)
    {

    }

    void CDebugLayer::OnAttach()
    {
        SGSINFO("On Attach: %s.", m_DebugName.c_str());
        CEngine::Get()->RequireImGui(true); 
    }

    void CDebugLayer::OnDetach()
    {
        SGSINFO("On Detach: %s.", m_DebugName.c_str());
        CEngine::Get()->RequireImGui(false);
    }

    void CDebugLayer::OnImGuiRender()
    {
        ImGui::Begin("Albarranco Engine!");
        ImGui::Text("This is Albarranco Engine.");
        ImGui::Text("Delta Time: %f.", CEngine::Get()->GetDeltaTime());
        ImGui::End();
    }

    void CDebugLayer::OnEvent(CEvent& aEvent)
    {
        if (!m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();

            aEvent.bHandled |= aEvent.IsInCategory(EVENT_CATEGORY_MOUSE) & io.WantCaptureMouse;
			aEvent.bHandled |= aEvent.IsInCategory(EVENT_CATEGORY_KEYBOARD) & io.WantCaptureKeyboard;
        }
    }
}