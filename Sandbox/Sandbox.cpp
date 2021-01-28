#include <stdio.h>

#include "Renderer.h"

int main()
{
    Window *window = OpenWindow(1920, 1080, "Sandbox App");

    RendererResult res = RendererInit();
    if (res != ResultSuccess)
    {
        printf("Failed to initialize renderer: %d\n", res);
        return 1;
    }

    while (IsWindowOpen(window))
    {
        PollWindowEvents(window);

        RendererBeginFrame();

        RenderQuad({ 100.0f, 200.0f, 50.0f, 50.0f }, { 0.8f, 0.2f, 0.2f, 0.5f });
        
        RendererEndFrame();
    }

    RendererShutdown();
    DestroyWindow(window);

    return 0;
}