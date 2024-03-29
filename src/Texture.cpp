#include "Texture.h"

#include "Renderer.h"
#include "Internal.h"
#include "Utils.h"

#include <stb_image.h>

Texture *CreateTexture(uint32_t width, uint32_t height)
{
    _Texture *texture = (_Texture *)calloc(1, sizeof(_Texture));

    texture->width = width;
    texture->height = height;

    texture->format = VK_FORMAT_R8G8B8A8_SRGB;

    _CreateTexture(texture, width, height, texture->format, nullptr, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkFramebufferCreateInfo fboInfo = {};
    fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fboInfo.pNext = nullptr;
    fboInfo.flags = 0;
    fboInfo.renderPass = renderer.toTexturePass;
    fboInfo.attachmentCount = 1;
    fboInfo.pAttachments = &texture->view;
    fboInfo.width = width;
    fboInfo.height = height;
    fboInfo.layers = 1;

    vkCreateFramebuffer(renderer.device, &fboInfo, nullptr, &texture->framebuffer);

    return (Texture *)texture;
}

Texture *LoadTextureFromPixels(uint32_t width, uint32_t height, uint8_t *pixels)
{
    _Texture *texture = (_Texture *)calloc(1, sizeof(_Texture));

    texture->width = width;
    texture->height = height;
    
    texture->format = VK_FORMAT_R8G8B8A8_SRGB;

    _CreateTexture(texture, width, height, texture->format, pixels, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    texture->framebuffer = VK_NULL_HANDLE;

    return (Texture *)texture;
}

Texture *LoadTextureFromFile(const char *filename)
{
    _Texture *texture = (_Texture *)calloc(1, sizeof(_Texture));

    int width, height, channels;
    stbi_uc *pixels = stbi_load(filename, &width, &height, &channels, 4);

    if (!pixels)
    {
        printf("Failed to load texture file: %s\n", filename);
        __debugbreak();
    }

    texture->width = (uint32_t)width;
    texture->height = (uint32_t)height;
    
    texture->format = VK_FORMAT_R8G8B8A8_SRGB;

    _CreateTexture(texture, texture->width, texture->height, texture->format, pixels, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    texture->framebuffer = VK_NULL_HANDLE;

    stbi_image_free(pixels);

    return (Texture *)texture;
}

void DestroyTexture(Texture *handle)
{
    vkDeviceWaitIdle(renderer.device);

    _Texture *texture = (_Texture *)handle;
    vmaDestroyImage(renderer.allocator, texture->image, texture->allocation);
    vkDestroyImageView(renderer.device, texture->view, nullptr);
    vkDestroySampler(renderer.device, texture->sampler, nullptr);

    if (texture->framebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(renderer.device, texture->framebuffer, nullptr);
}

void _CreateTexture(_Texture *texture, uint32_t width, uint32_t height, VkFormat format, uint8_t *pixels, VkImageUsageFlags usage)
{
    uint32_t formatMultiplier = format == VK_FORMAT_R8G8B8A8_SRGB ? 4 : 3;

    Buffer stagingBuffer = {};
    if (pixels != nullptr)
    {
        CreateBuffer(&stagingBuffer, width * height * formatMultiplier, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void *mem = MapBufferMemory(&stagingBuffer);
        memcpy(mem, pixels, width * height * formatMultiplier);
        UnmapBufferMemory(&stagingBuffer);
    }

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkCheck(vmaCreateImage(renderer.allocator, &imageInfo, &allocInfo, &texture->image, &texture->allocation, nullptr));

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.image = texture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.levelCount = 1;

    VkCheck(vkCreateImageView(renderer.device, &viewInfo, nullptr, &texture->view));

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(renderer.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkCheck(vkCreateSampler(renderer.device, &samplerInfo, nullptr, &texture->sampler));

    if (pixels != nullptr)
    {
        TransitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkCommandBuffer cmdBuffer = BeginSingleUseCommand();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.mipLevel = 0;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer.buffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        EndSingleUseCommand(cmdBuffer);

        TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vmaDestroyBuffer(renderer.allocator, stagingBuffer.buffer, stagingBuffer.allocation);
    }
    else
    {
        TransitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    texture->set = std::move(AllocateDescriptorSets(&renderer.texturePipeline, 1, 1)[0]);

    VkDescriptorImageInfo setImageInfo = {};
    setImageInfo.sampler = texture->sampler;
    setImageInfo.imageView = texture->view;
    setImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = texture->set;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &setImageInfo;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(renderer.device, 1, &write, 0, nullptr);
}

glm::vec2 TextureGetExtent(Texture *handle)
{
    _Texture *texture = (_Texture *)handle;

    return glm::vec2((float)texture->width, (float)texture->height);
}