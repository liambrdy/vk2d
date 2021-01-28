#include "Buffer.h"

#include "Internal.h"
#include "Utils.h"

void CreateBuffer(Buffer *buffer, uint32_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, bool createMapped /* = false */)
{
    buffer->size = size;
    buffer->usage = usage;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;
    if (createMapped)
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    VkCheck(vmaCreateBuffer(renderer.allocator, &bufferInfo, &allocInfo, &buffer->buffer, &buffer->allocation, nullptr));
}

void DestroyBuffer(Buffer *buffer)
{
    vmaDestroyBuffer(renderer.allocator, buffer->buffer, buffer->allocation);
}

void *MapBufferMemory(Buffer *buffer)
{
    void *mem;
    vmaMapMemory(renderer.allocator, buffer->allocation, &mem);

    return mem;
}

void UnmapBufferMemory(Buffer *buffer)
{
    vmaUnmapMemory(renderer.allocator, buffer->allocation);
}
