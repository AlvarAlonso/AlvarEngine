#pragma once

#include "core/defines.h"
#include <core/module_manager.hpp>
#include <core/layer_stack.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec2.hpp>

#include <functional>

struct GLFWwindow;

namespace Alvar
{
    class ILayer;
    class CDebugLayer;
    class CEvent;
    class CWindowCloseEvent;
    class CWindowResizeEvent;
    class CKeyPressedEvent;
    class CKeyReleasedEvent;
    class CMouseMovedEvent;
    class CMouseScrolledEvent;
    class CMouseButtonPressedEvent;
    class CMouseButtonReleasedEvent;
    class CRenderModule;

    namespace Input 
    {
        class CInputModule;
    }

    struct sWindowData
    {
        uint32_t Width;
        uint32_t Height;
        bool bVSync;
        std::function<void(CEvent&)> EventCallback;
    };

    class CEngine
    {
    public:
        static CEngine* Get();

        void StartUp();

        void Run();

        void Shutdown();

        void OnEvent(CEvent& aEvent);

        GLFWwindow* GetWindow();

        // TODO: Show only selected functionalities or find another way to share modules. Engine must have access to initialization and stuff like this
        // but probably other classes who wants to access a module should not have those kind of functions available.
        CRenderModule* GetRenderModule(){ return m_pRenderModule; }
        Input::CInputModule* GetInputModule(){ return m_pInputModule; }

        void PushLayer(ILayer* aLayer);
        void PushOverlay(ILayer* aLayer);

        // TODO: Maybe this can go to some Layer::sProperties struct so layers can ask for specific features from the engine when the engine pushes a layer.
        // Right now it is hardcoded so when a layer is detached and no longer needs ImGui the engine may shut it down. The engine should check wether other layers still need imgui before shut it down.
        void RequireImGui(bool aRequire); 

        float GetDeltaTime() const { return m_DeltaTime; }

        // TODO: Move to window class.
        glm::vec2 GetWindowCenter() const;
        void SetMouseCursor(const glm::vec2 aNewMousePosition);

        bool m_bFramebufferResized;

    private:
        CEngine();

        CEngine(const CEngine& aEngine) = delete;
        bool operator== (const CEngine& aEngine) = delete;

        void RegisterGLFWCallbacks();

        bool OnWindowClose(CWindowCloseEvent& aEvent);
        bool OnWindowResize(CWindowResizeEvent& aEvent);
        bool OnKeyPressed(CKeyPressedEvent& aEvent);
        bool OnKeyReleased(CKeyReleasedEvent& aEvent);
        bool OnMouseMoved(CMouseMovedEvent& aEvent);
        bool OnMouseScrolled(CMouseScrolledEvent& aEvent);
        bool OnMouseButtonPressed(CMouseButtonPressedEvent& aEvent);
        bool OnMouseButtonReleased(CMouseButtonReleasedEvent& aEvent);

        void InitImGui();

    private:
        static CEngine* m_pInstance;

        // TODO: Separate engine from application/window.
        GLFWwindow* m_pWindow;
        sWindowData m_WindowData;

        uint32 m_ClientWidth;
        uint32 m_ClientHeight;

        // TODO: Move time related stuff to a Time manager.
        float m_DeltaTime;

        CModuleManager m_ModuleManager;

        Input::CInputModule* m_pInputModule; // TODO: Ugly.
        CRenderModule* m_pRenderModule;

        CLayerStack m_LayerStack;
        CDebugLayer* m_DebugLayer;

        bool m_IsImGuiInitialized;
    };
}
