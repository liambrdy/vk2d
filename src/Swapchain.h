#pragma once

#include <volk.h>

#include <vector>

struct Swapchain
{
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    VkFormat imageFormat;
    VkColorSpaceKHR colorSpace;
    VkExtent2D extent;
    uint32_t imageCount;
    uint32_t currentImage;

    VkPresentModeKHR presentMode;

    std::vector<VkImage> images;
    std::vector<VkImageView> views;
};

void CreateSwapchain(Swapchain *swapchain, VkSurfaceKHR surface);
void DestroySwapchain(Swapchain *swapchain);

VkResult AcquireNextImage(Swapchain *swapchain, uint32_t *currentImage, VkSemaphore imageAvailable);
VkResult PresentImage(Swapchain *swapchain, VkSemaphore waitSemaphore);