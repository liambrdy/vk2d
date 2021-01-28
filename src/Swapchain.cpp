#include "Swapchain.h"

#include "Internal.h"
#include "Utils.h"

void CreateSwapchain(Swapchain *swapchain, VkSurfaceKHR surface)
{
    bool recreate = true;

    if (swapchain->swapchain == VK_NULL_HANDLE)
    {
        recreate = false;

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(renderer.physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(renderer.physicalDevice, surface, &formatCount, formats.data());

        bool found = false;
        for (uint32_t i = 0; i < formatCount; ++i)
        {
            if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                swapchain->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
                swapchain->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

                found = true;

                break;
            }
        }

        if (!found)
        {
            swapchain->imageFormat = formats[0].format;
            swapchain->colorSpace = formats[0].colorSpace;
        }

        uint32_t presentCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(renderer.physicalDevice, surface, &presentCount, nullptr);
        std::vector<VkPresentModeKHR> presents(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(renderer.physicalDevice, surface, &presentCount, presents.data());

        for (uint32_t i = 0; i < presentCount; ++i)
        {
            if (presents[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain->presentMode = presents[i];
            }
            else if (presents[i] == VK_PRESENT_MODE_FIFO_KHR)
            {
                swapchain->presentMode = presents[i];
            }
        }
    }

    VkSurfaceCapabilitiesKHR caps = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer.physicalDevice, surface, &caps);

    swapchain->imageCount = caps.minImageCount + 1;

    swapchain->extent = caps.currentExtent;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = swapchain->imageCount;
    swapchainInfo.imageFormat = swapchain->imageFormat;
    swapchainInfo.imageColorSpace = swapchain->colorSpace;
    swapchainInfo.imageExtent = swapchain->extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = swapchain->presentMode;
    swapchainInfo.clipped = VK_TRUE;

    VkSwapchainKHR oldSwapchain = swapchain->swapchain;
    swapchainInfo.oldSwapchain = oldSwapchain;

    VkCheck(vkCreateSwapchainKHR(renderer.device, &swapchainInfo, nullptr, &swapchain->swapchain));
    
    if (recreate)
    {
        vkDestroySwapchainKHR(renderer.device, oldSwapchain, nullptr);

        for (uint32_t i = 0; i < swapchain->imageCount; ++i)
        {
            vkDestroyImageView(renderer.device, swapchain->views[i], nullptr);
        }
    }

    vkGetSwapchainImagesKHR(renderer.device, swapchain->swapchain, &swapchain->imageCount, nullptr);
    swapchain->images.resize(swapchain->imageCount);
    vkGetSwapchainImagesKHR(renderer.device, swapchain->swapchain, &swapchain->imageCount, swapchain->images.data());

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapchain->imageFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.levelCount = 1;

    swapchain->views.resize(swapchain->imageCount);
    for (uint32_t i = 0; i < swapchain->imageCount; ++i)
    {
        viewInfo.image = swapchain->images[i];

        vkCreateImageView(renderer.device, &viewInfo, nullptr, &swapchain->views[i]);
    }
}

void DestroySwapchain(Swapchain *swapchain)
{
    vkDestroySwapchainKHR(renderer.device, swapchain->swapchain, nullptr);

    for (uint32_t i = 0; i < swapchain->imageCount; ++i)
    {
        vkDestroyImageView(renderer.device, swapchain->views[i], nullptr);
    }
}

VkResult AcquireNextImage(Swapchain *swapchain, uint32_t *currentImage, VkSemaphore imageAvailable)
{
    VkResult res = vkAcquireNextImageKHR(renderer.device, swapchain->swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, currentImage);
    swapchain->currentImage = *currentImage;

    return res;
}

VkResult PresentImage(Swapchain *swapchain, VkSemaphore waitSemaphore)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapchain;
    presentInfo.pImageIndices = &swapchain->currentImage;
    presentInfo.pResults = nullptr;

    return vkQueuePresentKHR(renderer.queue, &presentInfo);
}
