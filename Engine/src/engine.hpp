#pragma once

#include "core/defines.h"

#include "renderer/render_module.hpp"

struct GLFWwindow;

class CEngine
{
public:
    static CEngine* Get();

    void StartUp();

    void Run();

    void Shutdown();

    GLFWwindow* GetWindow();

    // TODO: Show only selected functionalities or find another way to share modules. Engine must have access to initialization and stuff like this
    // but probably other classes who wants to access a module should not have those kind of functions available.
    CRenderModule* GetRenderModule(){ return &m_RenderModule; }

    float GetDeltaTime();

    bool m_bFramebufferResized;

private:
    CEngine();

    CEngine(const CEngine& aEngine) = delete;
    bool operator== (const CEngine& aEngine) = delete;

private:
    static CEngine* m_pInstance;

    GLFWwindow* m_pWindow;

    uint32 m_ClientWidth = 800;
    uint32 m_ClientHeight = 600;

    // TODO: Move time related stuff to a Time manager.
    float m_DeltaTime;

    CRenderModule m_RenderModule;
};