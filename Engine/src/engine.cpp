#include "engine.hpp"
#include "core/logger.h"

#include <GLFW/glfw3.h>

#include <chrono>

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

GLFWwindow* CEngine::GetWindow()
{
    return  m_pWindow;
}

float CEngine::GetDeltaTime()
{
    return m_DeltaTime;
}
