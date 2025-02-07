#include "engine.hpp"
#include "core/logger.h"
#include <core/event_system/event_base.hpp>
#include <core/event_system/application_events.hpp>
#include <debug/debug_layer.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <chrono>

namespace Alvar
{
    CEngine* CEngine::m_pInstance = nullptr;

    CEngine::CEngine() : 
        m_bFramebufferResized(false), m_pWindow(nullptr), m_WindowData(), 
        m_ClientWidth(800), m_ClientHeight(600),
        m_DeltaTime(0.0f), m_InputModule(), m_RenderModule(),
        m_LayerStack(), m_DebugLayer(nullptr),
        m_IsImGuiInitialized(false)

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
        m_InputModule.Initialize();
        m_RenderModule.Initialize();

        // TODO: Initialize layers.
        m_DebugLayer = new CDebugLayer();
        PushOverlay(m_DebugLayer);
    }

    void CEngine::Run()
    {
        while (!glfwWindowShouldClose(m_pWindow))
        {
            glfwPollEvents();

            auto Start = std::chrono::system_clock::now();

            m_InputModule.Update(m_DeltaTime);
            m_RenderModule.Update(m_DeltaTime);
        
            auto End = std::chrono::system_clock::now();
            auto Elapsed = std::chrono::duration_cast<std::chrono::microseconds>(End - Start);
            m_DeltaTime = Elapsed.count() / 1000.f;
        }
    }

    void CEngine::Shutdown()
    {
        SGSINFO("Shutdown!");

        // TODO: Pop layers.

        m_RenderModule.Shutdown();
        m_InputModule.Shutdown();

        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

    void CEngine::OnEvent(CEvent& aEvent)
    {
        // TODO: Dispatch the events in each module?
        CEventDispatcher Dispatcher(aEvent);
        Dispatcher.Dispatch<CWindowCloseEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnWindowClose));
        Dispatcher.Dispatch<CWindowResizeEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnWindowResize));

        // TODO: If event is unhandled, forward the event to the layers.
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (aEvent.bHandled)
                break;
            (*it)->OnEvent(aEvent);
        }

        // TODO: mmm...
        Dispatcher.Dispatch<CKeyPressedEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnKeyPressed));
        Dispatcher.Dispatch<CKeyReleasedEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnKeyReleased));
        Dispatcher.Dispatch<CMouseMovedEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnMouseMoved));
        Dispatcher.Dispatch<CMouseScrolledEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnMouseScrolled));
        Dispatcher.Dispatch<CMouseButtonPressedEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnMouseButtonPressed));
        Dispatcher.Dispatch<CMouseButtonReleasedEvent>(ALVAR_BIND_EVENT_FN(CEngine::OnMouseButtonReleased));
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

    bool CEngine::OnKeyPressed(CKeyPressedEvent& aEvent)
    {
        return m_InputModule.HandleKeyPressed(aEvent);
    }

    bool CEngine::OnKeyReleased(CKeyReleasedEvent& aEvent)
    {
        return m_InputModule.HandleKeyReleased(aEvent);
    }

    bool CEngine::OnMouseMoved(CMouseMovedEvent& aEvent)
    {
        return m_InputModule.HandleMouseMoved(aEvent);
    }

    bool CEngine::OnMouseScrolled(CMouseScrolledEvent& aEvent)
    {
        return m_InputModule.HandleMouseScrolled(aEvent);
    }

    bool CEngine::OnMouseButtonPressed(CMouseButtonPressedEvent& aEvent)
    {
        return m_InputModule.HandleMouseButtonPressed(aEvent);
    }

    bool CEngine::OnMouseButtonReleased(CMouseButtonReleasedEvent& aEvent)
    {
        return m_InputModule.HandleMouseButtonReleased(aEvent);
    }

    GLFWwindow* CEngine::GetWindow()
    {
        return  m_pWindow;
    }

    void CEngine::PushLayer(ILayer* aLayer)
    {
		m_LayerStack.PushLayer(aLayer);
		aLayer->OnAttach();
    }

    void CEngine::PushOverlay(ILayer* aLayer)
    {
		m_LayerStack.PushOverlay(aLayer);
		aLayer->OnAttach();
    }

    void CEngine::RequireImGui(bool aRequire)
    {
        if (aRequire && !m_IsImGuiInitialized)
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

            m_RenderModule.InitImGuiBackend();

            m_IsImGuiInitialized = true;
        }
        else if (!aRequire && m_IsImGuiInitialized)
        {
            m_RenderModule.ShutdownImGuiBackend();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            m_IsImGuiInitialized = false;
        }
    }
}
