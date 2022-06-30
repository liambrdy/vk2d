#pragma once

#include <vector>
#include <deque>
#include <functional>

#include <volk.h>
#include <vk_mem_alloc.h>

#ifdef _WIN32
#include "Win32Platform.h"
#endif

#include "Swapchain.h"
#include "GraphicsPipeline.h"
#include "Shader.h"
#include "Buffer.h"
#include "Texture.h"

#include "Renderer.h"

#include "Tracy.hpp"
#include "TracyVulkan.hpp"

struct FrameResources
{
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    VkFence renderFinishedFence;

    VkCommandBuffer commandBuffer;

    VkDescriptorSet frameUBO;
    Buffer frameBuffer;
};

struct _Window
{
    uint32_t width, height;
    const char *title;

    bool shouldClose;

    PLATFORM_WINDOW;
};

struct Renderer
{
    bool initialized;
    bool debug;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue queue;
    uint32_t graphicsQueueIndex;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    _Window *currentWindow;
    VkSurfaceKHR surface;

    VkRenderPass renderPass;
    VkRenderPass toTexturePass;
    VkRenderPass midRenderPass;
    
    Swapchain swapchain;
    std::vector<FrameResources> frames;
    std::vector<VkFramebuffer> framebuffers;
    uint32_t frameIndex;
    uint32_t currentImage;

    Texture *currentTarget;

    VkPipelineCache cache;
    GraphicsPipeline texturePipeline;
    GraphicsPipeline colorQuadPipeline;
    GraphicsPipeline linePipeline;

    VmaAllocator allocator;
    TracyVkCtx ctx;

    Buffer quadVertexBuffer;
    Buffer lineVertexBuffer;

    Buffer *lastBuffer;
    GraphicsPipeline *lastPipeline;
    uint64_t lastSetHash;

    VkResult result;

    std::deque<std::function<void ()>> deletionQueue;

    std::vector<const char *> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
    };

    std::vector<const char *> layerNames = {
        "VK_LAYER_KHRONOS_validation"
    };
};

extern Renderer renderer;

void PlatformCreateWindow(_Window *window);
void PlatformDestroyWindow(_Window *window);

void PlatformPollWindowEvents(_Window *window);

VkSurfaceKHR PlatformGetSurface(_Window *window);