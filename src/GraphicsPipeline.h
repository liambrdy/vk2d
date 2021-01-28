#pragma once

#include <volk.h>

#include "Shader.h"

struct GraphicsPipeline
{
    VkPipelineLayout layout;
    VkPipeline pipeline;

    std::vector<VkDescriptorSetLayout> setLayouts;
};

void CreateGraphicsPipeline(GraphicsPipeline *pipeline, Shader *shader);
void DestroyGraphicsPipeline(GraphicsPipeline *pipeline);