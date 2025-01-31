#include "engine.hpp"
#include "core/logger.h"
#include <core/event_system/event_base.hpp>
#include <core/event_system/application_events.hpp>

#include <GLFW/glfw3.h>

#include <chrono>

namespace Alvar
{
    CEngine* CEngine::m_pInstance = nullptr;

    CEngine::CEngine() : m_bFramebufferResized(false), m_pWindow(nullptr), m_DeltaTime(0.0f)
    {
        SGSINFO("Engine object created!");
    }

    // TODO: Game Engine Architecture book describes a cleaner way to do that.
    CEngine *CEngine::Get()
    {
        if (m_pInstance == nullptr)
        {
            m_pInstance = new CEngine();
        }

        return m_pInstance;
    }

    void CEngine::StartUp()
    {
        SGSINFO("StartUp!");
        
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        m_pWindow = glfwCreateWindow(m_ClientWidth, m_ClientHeight, "AlvarEngine", nullptr, nullptr);
        glfwSetWindowUserPointer(m_pWindow, this);
        glfwSetFramebufferSizeCallback(m_pWindow, [](GLFWwindow* aWindow, int aWidth, int aHeight)
        {
            auto App = reinterpret_cast<CEngine*>(glfwGetWindowUserPointer(aWindow));
            App->GetRenderModule()->HandleWindowResize(); // TODO: Refactor this.
        });

        RegisterGLFWCallbacks();

        m_EventCallback = ALVAR_BIND_EVENT_FN(OnEvent);

        // Initialize modules.
        m_RenderModule.Initialize();

        // TODO: Initialize layers.
    }

    void CEngine::Run()
    {
        while (!glfwWindowShouldClose(m_pWindow))
        {
            auto Start = std::chrono::system_clock::now();

            glfwPollEvents();
            m_RenderModule.Update();
        
            auto End = std::chrono::system_clock::now();
            auto Elapsed = std::chrono::duration_cast<std::chrono::microseconds>(End - Start);
            m_DeltaTime = Elapsed.count() / 1000.f;
        }
    }

    void CEngine::Shutdown()
    {
        SGSINFO("Shutdown!");
        m_RenderModule.Shutdown();

        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

    void CEngine::OnEvent(CEvent& aEvent)
    {
        CEventDispatcher Dispatcher(aEvent);
        Dispatcher.Dispatch<CWindowCloseEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnWindowClose));
        Dispatcher.Dispatch<CWindowResizeEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnWindowResize));

        // TODO: If event is unhandled, forward the event to the layers.
    }

    void CEngine::RegisterGLFWCallbacks()
    {

    }

    bool CEngine::OnWindowClose(CWindowCloseEvent& aEvent)
    {
        // TODO: Close main loop.
        return true;
    }

    bool CEngine::OnWindowResize(CWindowResizeEvent& aEvent)
    {
        // TODO: Forward this to renderer.
        return true;
    }

    GLFWwindow* CEngine::GetWindow()
    {
        return  m_pWindow;
    }

    float CEngine::GetDeltaTime()
    {
        return m_DeltaTime;
    }
}
