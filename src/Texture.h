#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <glm/glm.hpp>

struct _Texture
{
    VkImage image;
    VmaAllocation allocation;

    VkImageView view;
    VkSampler sampler;

    VkDescriptorSet set;

    uint32_t width, height;
    VkFormat format;
};

void _CreateTexture(_Texture *texture, uint32_t width, uint32_t height, VkFormat format, uint8_t *pixels);