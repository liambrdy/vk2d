#include "Internal.h"

#include "Utils.h"

#include <Windows.h>
#include <windowsx.h>

static void WindowCloseRequested(_Window *window)
{
    window->shouldClose = true;
}

_Window *GetWindowHandle(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    _Window *handle;
    if (msg == WM_NCCREATE)
    {
        LPCREATESTRUCT createStruct = (LPCREATESTRUCT)lParam;
        handle = (_Window *)createStruct->lpCreateParams;
        SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)handle);
    }
    else
    {
        handle = (_Window *)GetWindowLongPtr(window, GWLP_USERDATA);
    }

    return handle;
}

LRESULT CALLBACK WindowProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    _Window *window = GetWindowHandle(wnd, message, wParam, lParam);

    switch (message)
    {
        case WM_CLOSE:
        {
            WindowCloseRequested(window);
        } break;

        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
        } break;

        default:
        {
            result =  DefWindowProc(wnd, message, wParam, lParam);
        }
    }

    return result;
}

static void RegisterWindowCLassWin32()
{
    WNDCLASS windowClass = {};
    windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.lpszClassName = "DefaultClassName";
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&windowClass);
}

void PlatformCreateWindow(_Window *window)
{
    RegisterWindowCLassWin32();

    uint32_t realWidth = window->width;
    uint32_t realHeight = window->height;

    RECT rect = { 0, 0, (LONG)realWidth, (LONG)realHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

    realWidth = rect.right - rect.left;
    realHeight = rect.bottom - rect.top;

    window->win32.window = CreateWindow("DefaultClassName", window->title, WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, realWidth, realHeight, NULL, NULL, GetModuleHandle(NULL), (void *)window);
}

void PlatformDestroyWindow(_Window *window)
{
    DestroyWindow(window->win32.window);
}

void PlatformPollWindowEvents(_Window *window)
{
    MSG msg;
    while (PeekMessage(&msg, window->win32.window, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            WindowCloseRequested(window);
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

VkSurfaceKHR PlatformGetSurface(_Window *window)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.hwnd = window->win32.window;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkCheck(vkCreateWin32SurfaceKHR(renderer.instance, &surfaceInfo, nullptr, &surface));

    return surface;
}