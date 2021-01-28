#pragma once

#include <vector>

#include <volk.h>

struct DescriptorSetData
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    uint32_t setIndex;
};

struct Shader
{
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    std::vector<VkVertexInputAttributeDescription> descriptions;
    VkVertexInputBindingDescription binding;

    std::vector<DescriptorSetData> sets;
    std::vector<VkPushConstantRange> ranges;
};

void CreateShader(Shader *shader, const char *vertPath, const char *fragPath);
void DestroyShader(Shader *shader);