#include "Renderer.h"

#include "Internal.h"

Window *OpenWindow(uint32_t width, uint32_t height, const char *title)
{
    _Window *window = (_Window *)calloc(1, sizeof(_Window));

    window->width = width;
    window->height = height;
    window->title = title;

    window->shouldClose = false;

    PlatformCreateWindow(window);

    renderer.currentWindow = window;

    return (Window *)window;
}

void DestroyWindow(Window *handle)
{
    _Window *window = (_Window *)handle;

    PlatformDestroyWindow(window);

    free(handle);
}

bool IsWindowOpen(Window *handle)
{
    _Window *window = (_Window *)handle;

    return !window->shouldClose;
}

void PollWindowEvents(Window *handle)
{
    _Window *window = (_Window *)handle;

    PlatformPollWindowEvents(window);
}