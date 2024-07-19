#include "engine.hpp"
#include "core/logger.h"

#include <GLFW/glfw3.h>

CEngine* CEngine::m_pInstance = nullptr;

CEngine::CEngine() : m_bFramebufferResized(false), m_pWindow(nullptr)
{
    SGSINFO("Engine object created!");
}

CEngine *CEngine::Get()
{
    if (m_pInstance == nullptr)
    {
        m_pInstance = new CEngine();
    }

    return m_pInstance;
}

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<CEngine*>(glfwGetWindowUserPointer(window));
    app->m_bFramebufferResized = true;
}

void CEngine::StartUp()
{
    SGSINFO("StartUp!");
    
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    m_pWindow = glfwCreateWindow(m_ClientWidth, m_ClientHeight, "AlvarEngine", nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);

    m_RenderModule.Initialize();
}

void CEngine::Run()
{
    while (!glfwWindowShouldClose(m_pWindow))
    {
        glfwPollEvents();
        m_RenderModule.Render();
    }

    return;
}

void CEngine::Shutdown()
{
    SGSINFO("Shutdown!");
    m_RenderModule.Shutdown();

    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

GLFWwindow* CEngine::GetWindow()
{
    return  m_pWindow;
}
