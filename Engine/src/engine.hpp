#pragma once

#include "core/defines.h"

#include <windows.h>

class Engine
{
public:
    static Engine* Get();

    void StartUp();

    void Run();

    void Shutdown();

    LRESULT WinProc(HWND aWindowHandle, UINT aMsg, WPARAM aWParam, LPARAM aLParam);

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
};