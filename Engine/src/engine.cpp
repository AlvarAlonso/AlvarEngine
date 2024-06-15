#include "engine.hpp"
#include "core/logger.h"

Engine* Engine::m_pInstance = nullptr;

LRESULT CALLBACK MainWindowProcedure(HWND aWindowHandle, UINT aMsg, WPARAM aWParam, LPARAM aLParam)
{
    return Engine::Get()->WinProc(aWindowHandle, aMsg, aWParam, aLParam);
}

Engine::Engine()
{
    SGSINFO("Engine object created!");
}

Engine *Engine::Get()
{
    if (m_pInstance == nullptr)
    {
        m_pInstance = new Engine();
    }

    return m_pInstance;
}

void Engine::StartUp()
{
    SGSINFO("StartUp!");
    InitWindowsApp();
    
    m_VulkanModule.Initialize(m_WindowsInstance, m_WindowHandle);
}

void Engine::Run()
{
    MSG msg = {0};

    // TODO: does not quit when clicking the cross.
    while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
		}

        m_VulkanModule.Render();
    }

    return;
}

void Engine::Shutdown()
{
    SGSINFO("Shutdown!");
    m_VulkanModule.Shutdown();
}

bool Engine::InitWindowsApp()
{
    m_WindowsInstance = GetModuleHandleA(0);
    // TODO: Should check m_WindowsInstance validity?
    HICON icon = LoadIcon(m_WindowsInstance, IDI_APPLICATION);
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProcedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_WindowsInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "WindowClass";

    // Register Window Class
    if (!RegisterClass(&wc))
    {
        MessageBox(0, "RegisterClass FAILED!!!", 0, 0);
        return false;
    }

    // Compute window dimensions
    RECT Rect = {0, 0, (LONG)m_ClientWidth, (LONG)m_ClientHeight};
    AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, false);
    LONG Width = Rect.right - Rect.left;
    LONG Height = Rect.bottom - Rect.top;

    // Create Window
    m_WindowHandle =  CreateWindow(
        "WindowClass",
        "AlvarEngine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        Width,
        Height,
        0,
        0,
        m_WindowsInstance,
        0);

    if (m_WindowHandle == 0)
    {
        MessageBox(0, "CreateWindow FAILED!!!", 0, 0);
        return false;
    }

    ShowWindow(m_WindowHandle, true);
    UpdateWindow(m_WindowHandle);

    return true;
}

LRESULT CALLBACK Engine::WinProc(HWND aWindowHandle, UINT aMsg, WPARAM aWParam, LPARAM aLParam)
{
    switch (aMsg)
    {
        // Sent when the window is activated or deactivated.
        case WM_ACTIVATE:
            if(LOWORD(aWParam) == WA_INACTIVE)
            {
                // TODO: Pause game.
            }
            else
            {
                // TODO: Unpause game.
            }
            return 0;
        // Sent when the user resizes the window.
        case WM_SIZE:
            m_ClientWidth = LOWORD(aLParam);
            m_ClientHeight = HIWORD(aLParam);

            if(aWParam == SIZE_MINIMIZED)
            {
                // TODO: Handle minimization of window. Pause game, etc.
            }
            else if (aWParam == SIZE_MAXIMIZED)
            {
                // TODO: Handle maximization of window. Unpause game, etc.
            }
            else if (aWParam == SIZE_RESTORED)
            {
                // TODO: Handle restoration of window, from minimized, maximized or resizing state.
            }
            else
            {
                // TODO: Handle resizing.
            }

            return 0;
        break;
        // Sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            // TOOD:
            // Pause game
            // Handle resizing
            return 0;
        // Sent when the user releases the resize bars.
        case WM_EXITSIZEMOVE:
            // TODO:
            // Handle resizing.
            return 0;
        // Prevent window from becoming to small.
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)aLParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)aLParam)->ptMinTrackSize.y = 200;
            return 0;
        
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            // TODO: Handle mouse button down.
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            // TODO: Handle mouse button up.
            return 0;
        case WM_MOUSEMOVE:
            // TODO: Handle mouse move.
            return 0;
        case WM_KEYUP:
            if(aWParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }

            return 0;
        default:
            return DefWindowProcA(aWindowHandle, aMsg, aWParam, aLParam);
    }
}
