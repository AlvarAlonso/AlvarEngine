#pragma once

#include "core/defines.h"

#include <windows.h>

#include "renderer/render_module.hpp"
struct sDimension2D
{
    uint32_t Width = 0;
    uint32_t Height = 0;
};

class Engine
{
public:
    static Engine* Get();

    void StartUp();

    void Run();

    void Shutdown();

    LRESULT WinProc(HWND aWindowHandle, UINT aMsg, WPARAM aWParam, LPARAM aLParam);

    // TODO: I need a way to export only some functions. I don't want all public functions to be exported to the sandbox.
    sDimension2D GetAppWindowDimensions();

    // TODO: Show only selected functionalities or find another way to share modules. Engine must have access to initialization and stuff like this
    // but probably other classes who wants to access a module should not have those kind of functions available.
    CRenderModule* GetRenderModule(){ return &m_RenderModule; }

private:
    Engine();

    Engine(const Engine& aEngine) = delete;
    bool operator== (const Engine& aEngine) = delete;

    bool InitWindowsApp();

private:
    static Engine* m_pInstance;

    HINSTANCE m_WindowsInstance = 0;
    HWND m_WindowHandle = 0;

    uint32 m_ClientWidth = 800;
    uint32 m_ClientHeight = 600;

    CRenderModule m_RenderModule;
};