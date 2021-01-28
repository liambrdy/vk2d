#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

struct Buffer
{
    VkBuffer buffer;
    VmaAllocation allocation;

    uint32_t size;
    VkBufferUsageFlags usage;
};

void CreateBuffer(Buffer *buffer, uint32_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, bool createMapped = false);
void DestroyBuffer(Buffer *buffer);

void *MapBufferMemory(Buffer *buffer);
void UnmapBufferMemory(Buffer *buffer);