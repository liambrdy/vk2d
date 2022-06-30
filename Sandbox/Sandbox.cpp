#include <stdio.h>

#include "Renderer.h"

int main()
{
    Window *window = OpenWindow(1080, 720, "Sandbox App");

    RendererResult res = RendererInit();
    if (res != ResultSuccess)
    {
        printf("Failed to initialize renderer: %d\n", res);
        return 1;
    }

    Texture *offscreenTarget = CreateTexture(800, 600);

    while (IsWindowOpen(window))
    {
        PollWindowEvents(window);

        RendererBeginFrame();

        SetRenderTarget(offscreenTarget);

        RenderQuad({ 100.0f, 200.0f, 50.0f, 50.0f }, { 0.8f, 0.2f, 0.2f, 0.5f });
        RenderLine({ 400.0f, 400.0f }, { 300.0f, 300.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });

        SetRenderTarget(RENDER_TO_SCREEN);

        RenderTexture(offscreenTarget, { 200.0f, 200.0f, 800.0f / 2, 600.0f / 2 });
        
        RendererEndFrame();
    }

    DestroyTexture(offscreenTarget);

    RendererShutdown();
    DestroyWindow(window);

    return 0;
}