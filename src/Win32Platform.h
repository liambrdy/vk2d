#pragma once

#define PLATFORM_WINDOW Win32Window win32;

struct Win32Window
{
    HWND window;

    int lastCursorPosX, lastCursorPosY;
};