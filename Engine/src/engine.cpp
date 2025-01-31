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

        m_WindowData.Width = m_ClientWidth;
        m_WindowData.Height = m_ClientHeight;        
        m_pWindow = glfwCreateWindow(m_WindowData.Width, m_WindowData.Height, "AlvarEngine", nullptr, nullptr);

        glfwSetWindowUserPointer(m_pWindow, &m_WindowData);

        RegisterGLFWCallbacks();

        m_WindowData.EventCallback = ALVAR_BIND_EVENT_FN(OnEvent);

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
        glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* aWindow, int aWidth, int aHeight)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);
			Data.Width = aWidth;
			Data.Height = aHeight;

			CWindowResizeEvent Event(aWidth, aHeight);
			Data.EventCallback(Event);
		});

		glfwSetWindowCloseCallback(m_pWindow, [](GLFWwindow* aWindow)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);
			CWindowCloseEvent Event;
			Data.EventCallback(Event);
		});

		glfwSetKeyCallback(m_pWindow, [](GLFWwindow* aWindow, int aKey, int aScancode, int aAction, int aMods)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);

			switch (aAction)
			{
				case GLFW_PRESS:
				{
					CKeyPressedEvent Event(aKey, 0);
					Data.EventCallback(Event);
					break;
				}
				case GLFW_RELEASE:
				{
					CKeyReleasedEvent Event(aKey);
					Data.EventCallback(Event);
					break;
				}
				case GLFW_REPEAT:
				{
					CKeyPressedEvent Event(aKey, true);
					Data.EventCallback(Event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_pWindow, [](GLFWwindow* aWindow, unsigned int aKeycode)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);

			CKeyTypedEvent Event(aKeycode);
			Data.EventCallback(Event);
		});

		glfwSetMouseButtonCallback(m_pWindow, [](GLFWwindow* aWindow, int aButton, int aAction, int aMods)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);

			switch (aAction)
			{
				case GLFW_PRESS:
				{
					CMouseButtonPressedEvent Event(aButton);
					Data.EventCallback(Event);
					break;
				}
				case GLFW_RELEASE:
				{
					CMouseButtonReleasedEvent Event(aButton);
					Data.EventCallback(Event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_pWindow, [](GLFWwindow* aWindow, double xOffset, double yOffset)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);

			CMouseScrolledEvent Event((float)xOffset, (float)yOffset);
			Data.EventCallback(Event);
		});

		glfwSetCursorPosCallback(m_pWindow, [](GLFWwindow* aWindow, double xPos, double yPos)
		{
			sWindowData& Data = *(sWindowData*)glfwGetWindowUserPointer(aWindow);

			CMouseMovedEvent Event((float)xPos, (float)yPos);
			Data.EventCallback(Event);
		});
    }

    bool CEngine::OnWindowClose(CWindowCloseEvent& aEvent)
    {
        // TODO: Close main loop.
        return true;
    }

    bool CEngine::OnWindowResize(CWindowResizeEvent& aEvent)
    {
        m_RenderModule.HandleWindowResize();
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
