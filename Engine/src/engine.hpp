#pragma once

#include "core/defines.h"

#include <core/input/input_module.hpp>
#include "renderer/render_module.hpp"

#include <functional>

// TODO: Remove this from here.
#define ALVAR_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

struct GLFWwindow;

namespace Alvar
{
    class CEvent;
    class CWindowCloseEvent;
    class CWindowResizeEvent;
    class CKeyPressedEvent;
    class CKeyReleasedEvent;
    class CMouseMovedEvent;
    class CMouseScrolledEvent;
    class CMouseButtonPressedEvent;
    class CMouseButtonReleasedEvent;

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
        CRenderModule* GetRenderModule(){ return &m_RenderModule; }

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

    private:
        static CEngine* m_pInstance;

        GLFWwindow* m_pWindow;
        sWindowData m_WindowData;

        uint32 m_ClientWidth = 800;
        uint32 m_ClientHeight = 600;

        // TODO: Move time related stuff to a Time manager.
        float m_DeltaTime;

        Input::CInputModule m_InputModule; // TODO: Ugly.
        CRenderModule m_RenderModule;
    };
}
