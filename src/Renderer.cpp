#include "Renderer.h"

#include "Internal.h"
#include "Utils.h"

#include <stb_image.h>

#include <assert.h>

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

Renderer renderer = { false };

#define QUAD_VERTEX_COUNT 6
std::vector<float> unitSquare = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f
};

#define LINE_VERTEX_COUNT 2
std::vector<float> unitLine = {
    0.0f, 0.0f, 1.0f, 0.0f
};

VkBool32 VKAPI_PTR DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    printf("%s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

static bool CheckInstanceSupport()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> props(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, props.data());

    for (int i = 0; i < renderer.instanceExtensions.size(); ++i)
    {
        const char *name = renderer.instanceExtensions[i];
        bool found = false;

        for (int j = 0; j < props.size(); ++j)
        {
            if (strcmp(props[j].extensionName, name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}

static bool CheckInstanceDebugSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for (int i = 0; i < renderer.layerNames.size(); ++i)
    {
        const char *name = renderer.layerNames[i];
        bool found = false;

        for (int j = 0; j < layers.size(); ++j)
        {
            if (strcmp(layers[j].layerName, name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    renderer.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return true;
}

static bool IsDeviceSuitable(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> props(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, props.data());

    for (int i = 0; i < renderer.deviceExtensions.size(); ++i)
    {
        const char *name = renderer.deviceExtensions[i];
        bool found = false;

        for (int j = 0; j < props.size(); ++j)
        {
            if (strcmp(props[j].extensionName, name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    if (renderer.debug)
    {
        uint32_t layerCount = 0;
        vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateDeviceLayerProperties(device, &layerCount, layers.data());

        for (int i = 0; i < renderer.layerNames.size(); ++i)
        {
            const char *name = renderer.layerNames[i];
            bool found = false;

            for (int j = 0; j < layers.size(); ++j)
            {
                if (strcmp(layers[j].layerName, name) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return false;
        }
    }
    
    return true;
}

RendererResult RendererInit()
{
    ZoneScopedN("Engine initialization");

    if (renderer.initialized)
        return ResultUnknown;

    renderer.initialized = true;

    if (!renderer.currentWindow)
        return ResultNoWindow;

#ifdef NDEBUG
    renderer.debug = false;
#else
    renderer.debug = true;
#endif

    stbi_set_flip_vertically_on_load(true);

    renderer.result = volkInitialize();
    if (renderer.result == VK_ERROR_INITIALIZATION_FAILED)
        return ResultNoGpu;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};

    if (renderer.debug)
    {
        CheckInstanceDebugSupport();

        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext = nullptr;
        debugInfo.flags = 0;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = DebugCallback;
        debugInfo.pUserData = nullptr;
    }

    if (!CheckInstanceSupport())
        return ResultExtensionsNotFound;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Chess";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.enabledExtensionCount = (uint32_t)renderer.instanceExtensions.size();
    instanceInfo.ppEnabledExtensionNames = renderer.instanceExtensions.data();
    if (renderer.debug)
    {
        instanceInfo.pNext = &debugInfo;
        instanceInfo.enabledLayerCount = (uint32_t)renderer.layerNames.size();
        instanceInfo.ppEnabledLayerNames = renderer.layerNames.data();
    }
    else
    {
        instanceInfo.pNext = nullptr;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = nullptr;
    }
    instanceInfo.pApplicationInfo = &appInfo;

    VkCheck(vkCreateInstance(&instanceInfo, nullptr, &renderer.instance));
    if (renderer.result == VK_ERROR_EXTENSION_NOT_PRESENT)
        return ResultExtensionsNotFound;
    else if (renderer.result == VK_ERROR_LAYER_NOT_PRESENT)
        return ResultLayersNotFound;

    volkLoadInstanceOnly(renderer.instance);

    if (renderer.device)
    {
        VkCheck(vkCreateDebugUtilsMessengerEXT(renderer.instance, &debugInfo, nullptr, &renderer.debugMessenger));
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(renderer.instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(renderer.instance, &deviceCount, devices.data());

    renderer.physicalDevice = VK_NULL_HANDLE;

    for (int i = 0; i < devices.size(); ++i)
    {
        if (IsDeviceSuitable(devices[i]))
        {
            renderer.physicalDevice = devices[i];
            break;
        }
    }

    if (renderer.physicalDevice == VK_NULL_HANDLE)
        return ResultNoGpu;

    renderer.surface = PlatformGetSurface(renderer.currentWindow);

    VkPhysicalDeviceFeatures enabledFeatures = {};
    enabledFeatures.fillModeNonSolid = VK_TRUE;
    enabledFeatures.samplerAnisotropy = VK_TRUE;

    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(renderer.physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(renderer.physicalDevice, &queueCount, queueProps.data());

    for (int i = 0; i < queueProps.size(); ++i)
    {
        VkQueueFamilyProperties queue = queueProps[i];

        if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
        {
            renderer.graphicsQueueIndex = i;
        }
    }

    VkBool32 presentable;
    vkGetPhysicalDeviceSurfaceSupportKHR(renderer.physicalDevice, renderer.graphicsQueueIndex, renderer.surface, &presentable);
    if (!presentable)
        return ResultNoGpu;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.flags = 0;
    queueInfo.queueCount = 1;
    queueInfo.queueFamilyIndex = renderer.graphicsQueueIndex;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.flags = 0;
    deviceInfo.pEnabledFeatures = &enabledFeatures;
    deviceInfo.enabledExtensionCount = (uint32_t)renderer.deviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames = renderer.deviceExtensions.data();
    if (renderer.debug)
    {
        deviceInfo.enabledLayerCount = (uint32_t)renderer.layerNames.size();
        deviceInfo.ppEnabledLayerNames = renderer.layerNames.data();
    }
    else
    {
        deviceInfo.enabledLayerCount = 0;
        deviceInfo.ppEnabledLayerNames = nullptr;
    }
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    VkCheck(vkCreateDevice(renderer.physicalDevice, &deviceInfo, nullptr, &renderer.device));

    volkLoadDevice(renderer.device);

    vkGetDeviceQueue(renderer.device, renderer.graphicsQueueIndex, 0, &renderer.queue);

    VmaVulkanFunctions functions = {};
    functions.vkAllocateMemory = vkAllocateMemory;
    functions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    functions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
    functions.vkBindBufferMemory = vkBindBufferMemory;
    functions.vkBindImageMemory = vkBindImageMemory;
    functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    functions.vkCreateBuffer = vkCreateBuffer;
    functions.vkCreateImage = vkCreateImage;
    functions.vkDestroyBuffer = vkDestroyBuffer;
    functions.vkDestroyImage = vkDestroyImage;
    functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    functions.vkFreeMemory = vkFreeMemory;
    functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
    functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    functions.vkMapMemory = vkMapMemory;
    functions.vkUnmapMemory = vkUnmapMemory;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorInfo.instance = renderer.instance;
    allocatorInfo.physicalDevice = renderer.physicalDevice;
    allocatorInfo.device = renderer.device;
    allocatorInfo.pVulkanFunctions = &functions;

    VkCheck(vmaCreateAllocator(&allocatorInfo, &renderer.allocator));

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = renderer.graphicsQueueIndex;

    VkCheck(vkCreateCommandPool(renderer.device, &poolInfo, nullptr, &renderer.commandPool));

    renderer.deletionQueue.push_back([=]()
    {
        vkDestroyCommandPool(renderer.device, renderer.commandPool, nullptr);
    });

    VkCommandBuffer tracyCmdBuf = std::move(AllocateCommandBuffers(1)[0]);

    renderer.ctx = TracyVkContext(renderer.physicalDevice, renderer.device, renderer.queue, tracyCmdBuf);

    std::vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5 }
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.flags = 0;
    descriptorPoolInfo.maxSets = 10;
    descriptorPoolInfo.poolSizeCount = (uint32_t)sizes.size();
    descriptorPoolInfo.pPoolSizes = sizes.data();

    VkCheck(vkCreateDescriptorPool(renderer.device, &descriptorPoolInfo, nullptr, &renderer.descriptorPool));

    renderer.deletionQueue.push_back([=]()
    {
        vkDestroyDescriptorPool(renderer.device, renderer.descriptorPool, nullptr);
    });

    CreateSwapchain(&renderer.swapchain, renderer.surface);

    renderer.deletionQueue.push_back([=]()
    {
        DestroySwapchain(&renderer.swapchain);
    });

    uint32_t vertexSize = (uint32_t)unitSquare.size() * sizeof(float);
    CreateBuffer(&renderer.quadVertexBuffer, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    vertexSize = (uint32_t)unitSquare.size() * sizeof(float);
    CreateBuffer(&renderer.lineVertexBuffer, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    
    renderer.deletionQueue.push_back([&]()
    {
        DestroyBuffer(&renderer.lineVertexBuffer);
        DestroyBuffer(&renderer.quadVertexBuffer);
    });

    void *mem = MapBufferMemory(&renderer.quadVertexBuffer);
    memcpy(mem, unitSquare.data(), renderer.quadVertexBuffer.size);
    UnmapBufferMemory(&renderer.quadVertexBuffer);

    mem = MapBufferMemory(&renderer.lineVertexBuffer);
    memcpy(mem, unitLine.data(), renderer.lineVertexBuffer.size);
    UnmapBufferMemory(&renderer.lineVertexBuffer);

    // Default render pass initialization
    std::vector<VkAttachmentDescription> attachments(1);
    attachments[0].flags = 0;
    attachments[0].format = renderer.swapchain.imageFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderpassInfo = {};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.pNext = nullptr;
    renderpassInfo.flags = 0;
    renderpassInfo.attachmentCount = 1;
    renderpassInfo.pAttachments = attachments.data();
    renderpassInfo.subpassCount = 1;
    renderpassInfo.pSubpasses = &subpass;
    renderpassInfo.dependencyCount = 1;
    renderpassInfo.pDependencies = &dependency;

    vkCreateRenderPass(renderer.device, &renderpassInfo, nullptr, &renderer.renderPass);

    // Render to texture render pass initialization
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    vkCreateRenderPass(renderer.device, &renderpassInfo, nullptr, &renderer.toTexturePass);

    // Switch to this renderpass after rendering to texture
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    vkCreateRenderPass(renderer.device, &renderpassInfo, nullptr, &renderer.midRenderPass);

    renderer.deletionQueue.push_back([=]()
    {
        vkDestroyRenderPass(renderer.device, renderer.midRenderPass, nullptr);
        vkDestroyRenderPass(renderer.device, renderer.toTexturePass, nullptr);
        vkDestroyRenderPass(renderer.device, renderer.renderPass, nullptr);
    });

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = renderer.renderPass;
    framebufferInfo.width = renderer.swapchain.extent.width;
    framebufferInfo.height = renderer.swapchain.extent.height;
    framebufferInfo.layers = 1;

    renderer.framebuffers.resize(renderer.swapchain.imageCount);
    for (uint32_t i = 0; i < renderer.swapchain.imageCount; ++i)
    {
        std::vector<VkImageView> attachments = {
            renderer.swapchain.views[i]
        };

        framebufferInfo.attachmentCount = (uint32_t)attachments.size();
        framebufferInfo.pAttachments = attachments.data();

        vkCreateFramebuffer(renderer.device, &framebufferInfo, nullptr, &renderer.framebuffers[i]);

        renderer.deletionQueue.push_back([=]()
        {
            vkDestroyFramebuffer(renderer.device, renderer.framebuffers[i], nullptr);
        });
    }

    VkPipelineCacheCreateInfo cacheInfo = {};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cacheInfo.pNext = nullptr;
    cacheInfo.flags = 0;
    cacheInfo.initialDataSize = 0;
    cacheInfo.pInitialData = nullptr;

    VkCheck(vkCreatePipelineCache(renderer.device, &cacheInfo, nullptr, &renderer.cache));

    {
        Shader shader = {};
        CreateShader(&shader, "../../../res/shaders/texture.vert", "../../../res/shaders/texture.frag");

        CreateGraphicsPipeline(&renderer.texturePipeline, &shader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        DestroyShader(&shader);
    }

    {
        Shader shader = {};
        CreateShader(&shader, "../../../res/shaders/color.vert", "../../../res/shaders/color.frag");

        CreateGraphicsPipeline(&renderer.colorQuadPipeline, &shader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        CreateGraphicsPipeline(&renderer.linePipeline, &shader, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

        DestroyShader(&shader);
    }

    renderer.deletionQueue.push_back([=]()
    {
        DestroyGraphicsPipeline(&renderer.linePipeline);
        DestroyGraphicsPipeline(&renderer.colorQuadPipeline);
        DestroyGraphicsPipeline(&renderer.texturePipeline);
        vkDestroyPipelineCache(renderer.device, renderer.cache, nullptr);
    });

    renderer.frames.resize(renderer.swapchain.imageCount);
    renderer.frameIndex = 0;

    float aspect = (float)renderer.swapchain.extent.width / (float)renderer.swapchain.extent.height;
    glm::mat4 projection = glm::ortho(0.0f, (float)renderer.swapchain.extent.width, 0.0f, (float)renderer.swapchain.extent.height, 0.0f, 1.0f);

    for (uint32_t i = 0; i < renderer.frames.size(); ++i)
    {
        FrameResources &frame = renderer.frames[i];

        frame.renderFinishedFence = CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
        
        frame.imageAvailableSemaphore = CreateSemaphore();
        frame.renderFinishedSemaphore = CreateSemaphore();

        frame.commandBuffer = std::move(AllocateCommandBuffers(1)[0]);
        frame.frameUBO = std::move(AllocateDescriptorSets(&renderer.colorQuadPipeline, 1, 0)[0]);
        CreateBuffer(&frame.frameBuffer, sizeof(projection), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, false);

        void *mem = MapBufferMemory(&frame.frameBuffer);
        memcpy(mem, &projection[0][0], sizeof(projection));
        UnmapBufferMemory(&frame.frameBuffer);

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = frame.frameBuffer.buffer;
        bufferInfo.range = sizeof(projection);
        bufferInfo.offset = 0;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = frame.frameUBO;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pImageInfo = nullptr;
        write.pBufferInfo = &bufferInfo;
        write.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(renderer.device, 1, &write, 0, nullptr);

        renderer.deletionQueue.push_back([&]()
        {
            DestroyBuffer(&frame.frameBuffer);
        });
    }

    return ResultSuccess;
}

void RendererShutdown()
{
    vkDeviceWaitIdle(renderer.device);

    for (auto it = renderer.deletionQueue.rbegin(); it != renderer.deletionQueue.rend(); ++it)
    {
        (*it)();
    }

    renderer.deletionQueue.clear();

    vmaDestroyAllocator(renderer.allocator);

    TracyVkDestroy(renderer.ctx);

    vkDestroySurfaceKHR(renderer.instance, renderer.surface, nullptr);

    vkDestroyDevice(renderer.device, nullptr);
    if (renderer.device)
        vkDestroyDebugUtilsMessengerEXT(renderer.instance, renderer.debugMessenger, nullptr);
    vkDestroyInstance(renderer.instance, nullptr);    
}

static uint64_t HashSets(std::vector<VkDescriptorSet> sets)
{
    uint64_t hash = 0;
    for (uint32_t i = 0; i < sets.size(); ++i)
    {
        uint64_t *pointer = (uint64_t *)&sets[i];
        hash += (*pointer) + (i << 12);
    }

    return hash;
}

static void RecreateSwapchain()
{
    vkQueueWaitIdle(renderer.queue);

    CreateSwapchain(&renderer.swapchain, renderer.surface);

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = renderer.renderPass;
    framebufferInfo.width = renderer.swapchain.extent.width;
    framebufferInfo.height = renderer.swapchain.extent.height;
    framebufferInfo.layers = 1;

    for (uint32_t i = 0; i < renderer.swapchain.imageCount; ++i)
    {
        vkDestroyFramebuffer(renderer.device, renderer.framebuffers[i], nullptr);

        std::vector<VkImageView> attachments = {
            renderer.swapchain.views[i]
        };

        framebufferInfo.attachmentCount = (uint32_t)attachments.size();
        framebufferInfo.pAttachments = attachments.data();

        vkCreateFramebuffer(renderer.device, &framebufferInfo, nullptr, &renderer.framebuffers[i]);
    }
}

void RendererBeginFrame()
{
    ZoneScopedN("RendererBeginFrame");

    FrameResources &frame = renderer.frames[renderer.frameIndex];

    renderer.currentTarget = RENDER_TO_SCREEN;

    renderer.lastPipeline = nullptr;
    renderer.lastBuffer = nullptr;
    renderer.lastSetHash = 0;

    vkWaitForFences(renderer.device, 1, &frame.renderFinishedFence, true, UINT64_MAX);
    vkResetFences(renderer.device, 1, &frame.renderFinishedFence);

    renderer.result = AcquireNextImage(&renderer.swapchain, &renderer.currentImage, frame.imageAvailableSemaphore);

    vkResetCommandBuffer(frame.commandBuffer, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBeginInfo.pInheritanceInfo = nullptr;

    VkCheck(vkBeginCommandBuffer(frame.commandBuffer, &cmdBeginInfo));

    VkClearValue clearValue = {};
    clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderer.renderPass;
    beginInfo.framebuffer = renderer.framebuffers[renderer.currentImage];
    beginInfo.renderArea.extent = renderer.swapchain.extent;
    beginInfo.renderArea.offset = { 0, 0 };
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(frame.commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RendererEndFrame()
{
    ZoneScopedN("RendererEndFrame");

    FrameResources &frame = renderer.frames[renderer.frameIndex];

    vkCmdEndRenderPass(frame.commandBuffer);

    TracyVkCollect(renderer.ctx, frame.commandBuffer);

    VkCheck(vkEndCommandBuffer(frame.commandBuffer));

    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &dstStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderFinishedSemaphore;

    vkQueueSubmit(renderer.queue, 1, &submitInfo, frame.renderFinishedFence);

    renderer.result = PresentImage(&renderer.swapchain, frame.renderFinishedSemaphore);
    if (renderer.result == VK_SUBOPTIMAL_KHR || renderer.result == VK_ERROR_OUT_OF_DATE_KHR)
        RecreateSwapchain();

    renderer.frameIndex = (renderer.frameIndex + 1) % renderer.frames.size();
}

static void BackendRender(std::vector<VkDescriptorSet> sets, uint32_t vertexCount, Buffer *vertexBuffer, GraphicsPipeline *pipeline, glm::vec2 pos, glm::vec2 scale, glm::vec4 texCoord, glm::vec4 color)
{
    FrameResources &frame = renderer.frames[renderer.frameIndex];

    uint64_t hash = HashSets(sets);

    struct
    {
        glm::mat4 model;
        glm::vec4 texCoord;
        glm::vec4 color;
    } pushBuffer;
    pushBuffer.model = glm::translate(glm::mat4(1.0f), { pos.x, pos.y, 0.0f });
    pushBuffer.model = glm::scale(pushBuffer.model, { scale.x, scale.y, 0.0f });

    pushBuffer.texCoord = texCoord;
    pushBuffer.color = color;

    if (pipeline != renderer.lastPipeline)
    {
        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        renderer.lastPipeline = pipeline;
    }

    if (vertexBuffer != renderer.lastBuffer)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, &vertexBuffer->buffer, &offset);

        renderer.lastBuffer = vertexBuffer;
    }

    if (hash != renderer.lastSetHash)
    {
        vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, (uint32_t)sets.size(), sets.data(), 0, nullptr);

        renderer.lastSetHash = hash;
    }

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)renderer.swapchain.extent.width;
    viewport.height = (float)renderer.swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = renderer.swapchain.extent;

    vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);

    vkCmdPushConstants(frame.commandBuffer, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushBuffer), &pushBuffer);
    vkCmdDraw(frame.commandBuffer, vertexCount, 1, 0, 0);
}

void RenderQuad(glm::vec4 rect, glm::vec4 color)
{
    FrameResources &frame = renderer.frames[renderer.frameIndex];
    
    TracyVkZone(renderer.ctx, frame.commandBuffer, "RenderQuad");

    std::vector<VkDescriptorSet> sets = {
        frame.frameUBO
    };

    BackendRender(sets, QUAD_VERTEX_COUNT, &renderer.quadVertexBuffer, &renderer.colorQuadPipeline, { rect.x, rect.y }, { rect.z, rect.w }, {}, color);
}

void RenderTexture(Texture *handle, glm::vec4 rect, glm::vec4 texCoord, glm::vec4 color)
{
    _Texture *texture = (_Texture *)handle;

    FrameResources &frame = renderer.frames[renderer.frameIndex];

    TracyVkZone(renderer.ctx, frame.commandBuffer, "RenderTexture");

    std::vector<VkDescriptorSet> sets = {
        frame.frameUBO,
        texture->set
    };

    glm::vec4 coord = texCoord;

    BackendRender(sets, QUAD_VERTEX_COUNT, &renderer.quadVertexBuffer, &renderer.texturePipeline, { rect.x, rect.y }, { rect.z, rect.w }, coord, color);
}

void RenderLine(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
    FrameResources &frame = renderer.frames[renderer.frameIndex];

    std::vector<VkDescriptorSet> sets = {
        frame.frameUBO
    };

    BackendRender(sets, LINE_VERTEX_COUNT, &renderer.lineVertexBuffer, &renderer.linePipeline, pos, size, {}, color);
}

static void TransitionTargetImageLayout(_Texture *texture, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    FrameResources &frame = renderer.frames[renderer.frameIndex];

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destStage;

    if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    vkCmdPipelineBarrier(frame.commandBuffer, sourceStage, destStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void SetRenderTarget(Texture *texture)
{
    if (renderer.currentTarget == texture)
        return;

    FrameResources &frame = renderer.frames[renderer.frameIndex];

    vkCmdEndRenderPass(frame.commandBuffer);

    _Texture *tex = (_Texture *)texture;
    bool isScreen = texture == RENDER_TO_SCREEN;
    
    if (isScreen)
    {
        TransitionTargetImageLayout((_Texture *)renderer.currentTarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    else
    {
        TransitionTargetImageLayout(tex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    VkRenderPass currentPass = isScreen ? renderer.midRenderPass : renderer.toTexturePass;
    VkExtent2D extent = {};
    if (isScreen)
        extent = renderer.swapchain.extent;
    else
        extent = { tex->width, tex->height };

    VkClearValue clearValue = {};
    clearValue.color = { 1.0f, 0.0f, 0.0f, 1.0f };

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = currentPass;
    beginInfo.framebuffer = isScreen ? renderer.framebuffers[renderer.currentImage] : tex->framebuffer;
    beginInfo.renderArea.extent = extent;
    beginInfo.renderArea.offset = { 0, 0 };
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(frame.commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    renderer.currentTarget = texture;
}