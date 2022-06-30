#pragma once

#include <stdint.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

typedef struct Window Window;

Window *OpenWindow(uint32_t width, uint32_t height, const char *title);
void DestroyWindow(Window *window);

bool IsWindowOpen(Window *window);

void PollWindowEvents(Window *window);

enum RendererResult
{
    ResultSuccess,
    ResultNoWindow,
    ResultNoGpu,
    ResultExtensionsNotFound,
    ResultLayersNotFound,

    ResultUnknown
};

RendererResult RendererInit();
void RendererShutdown();

void RendererBeginFrame();
void RendererEndFrame();

typedef struct Texture Texture;

Texture *CreateTexture(uint32_t width, uint32_t height);
Texture *LoadTextureFromPixels(uint32_t width, uint32_t height, uint8_t *pixels);
Texture *LoadTextureFromFile(const char *filename);

glm::vec2 TextureGetExtent(Texture *handle);

void DestroyTexture(Texture *texture);

void RenderQuad(glm::vec4 rect, glm::vec4 color);
void RenderTexture(Texture *texture, glm::vec4 rect, glm::vec4 texCoord = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4 color = glm::vec4(1.0f));
void RenderLine(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

#define RENDER_TO_SCREEN (Texture *)nullptr

void SetRenderTarget(Texture *texture);