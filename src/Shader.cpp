#include "Shader.h"

#include "Internal.h"
#include "Utils.h"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <spirv_reflect.h>

#include <fstream>
#include <assert.h>
#include <string>
#include <thread>
#include <mutex>

static const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }
};

static std::mutex mutex;

static bool glslangInitialized = false;

static const std::string GetSuffix(const std::string &path)
{
    const size_t pos = path.rfind('.');
    return (pos == std::string::npos) ? "" : path.substr(pos + 1);
}

static const EShLanguage GetShaderStage(const std::string &stage)
{
    if (stage == "vert")
        return EShLangVertex;
    else if (stage == "frag")
        return EShLangFragment;
    else
    {
        assert(0 && "Unknown shader stage");
        return EShLangCount;
    }
}

static const std::vector<char> CompileToSpirv(const std::string &path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        printf("Failed to open file: %s\n", path.c_str());
        __debugbreak();
    }

    std::string inputglsl((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const char *inputcstr = inputglsl.c_str();

    EShLanguage shaderType = GetShaderStage(GetSuffix(path));
    glslang::TShader shader(shaderType);
    shader.setStrings(&inputcstr, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, 120);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_5);

    EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);

    size_t found = path.find_last_of("/\\");
    std::string noFilePath = path.substr(0, found);

    DirStackFileIncluder includer;
    includer.pushExternalLocalDirectory(noFilePath);

    TBuiltInResource resources;

    {
        std::lock_guard<std::mutex> gaurd(mutex);
        resources = DefaultTBuiltInResource;
    }

    // std::string preprocessedGLSL;
    // if (!shader.preprocess(&resources, 120, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
    // {
    //     printf("Failed to preprocess this file: %s\n", path.c_str());
    //     printf("%s\n", shader.getInfoLog());
    //     printf("%s\n", shader.getInfoDebugLog());

    //     __debugbreak();
    // }

    // const char *preprocessedCStr = preprocessedGLSL.c_str();
    // shader.setStrings(&preprocessedCStr, 1);

    if (!shader.parse(&resources, 120, false, messages))
    {
        printf("Failed to parse this file: %s\n", path.c_str());
        printf("%s\n", shader.getInfoLog());
        printf("%s\n", shader.getInfoDebugLog());
        
        __debugbreak();
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        printf("Failed to link this file: %s\n", path.c_str());
        printf("%s\n", program.getInfoLog());
        printf("%s\n", program.getInfoDebugLog());
        
        __debugbreak();
    }

    std::vector<uint32_t> spirv;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions options;
    options.validate = true;
    
    glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv, &logger, &options);

    std::vector<char> binarySpirv;
    binarySpirv.resize(sizeof(uint32_t) * spirv.size());

    memcpy(binarySpirv.data(), spirv.data(), sizeof(uint32_t) * spirv.size());

    return binarySpirv;
}

static uint32_t GetFormatSize(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R32_SFLOAT: return 1 * 4;
        case VK_FORMAT_R32G32_SFLOAT: return 2 * 4;
        case VK_FORMAT_R32G32B32_SFLOAT: return 3 * 4;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 4 * 4;
        default: return 0;
    }
}

static VkShaderModule CreateShaderModule(Shader *shader, std::vector<char> code)
{
    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.pNext = nullptr;
    moduleInfo.flags = 0;
    moduleInfo.codeSize = (uint32_t)code.size();
    moduleInfo.pCode = (uint32_t *)code.data();

    VkShaderModule shaderModule;
    VkCheck(vkCreateShaderModule(renderer.device, &moduleInfo, nullptr, &shaderModule));

    SpvReflectShaderModule spvModule;
    spvReflectCreateShaderModule(code.size(), code.data(), &spvModule);
    
    VkShaderStageFlagBits stage = (VkShaderStageFlagBits)spvModule.shader_stage;

    if (stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
    {
        uint32_t inCount = 0;
        spvReflectEnumerateInputVariables(&spvModule, &inCount, nullptr);
        std::vector<SpvReflectInterfaceVariable *> vars(inCount);
        spvReflectEnumerateInputVariables(&spvModule, &inCount, vars.data());

        uint32_t offset = 0;

        for (uint32_t i = 0; i < inCount; ++i)
        {
            SpvReflectInterfaceVariable &var = *(vars[i]);
            if (var.built_in == -1)
            {
                VkVertexInputAttributeDescription attribute = {};
                attribute.binding = 0;
                attribute.location = var.location;
                attribute.format = (VkFormat)var.format;
                attribute.offset = offset;

                shader->descriptions.push_back(attribute);

                offset += GetFormatSize(attribute.format);
            }
        }

        VkVertexInputBindingDescription &binding = shader->binding;
        binding.binding = 0;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        binding.stride = offset;
    }

    uint32_t setCount = 0;
    spvReflectEnumerateDescriptorSets(&spvModule, &setCount, nullptr);
    std::vector<SpvReflectDescriptorSet *> sets(setCount);
    spvReflectEnumerateDescriptorSets(&spvModule, &setCount, sets.data());

    uint32_t baseSet = (uint32_t)shader->sets.size();
    shader->sets.resize(setCount + baseSet);

    for (uint32_t i = 0; i < setCount; ++i)
    {
        SpvReflectDescriptorSet &set = *(sets[i]);

        DescriptorSetData &data = shader->sets[set.set];
        data.setIndex = set.set;
        data.bindings.resize(set.binding_count);

        for (uint32_t j = 0; j < set.binding_count; ++j)
        {
            SpvReflectDescriptorBinding &binding = *(set.bindings[j]);

            VkDescriptorSetLayoutBinding &setBinding = data.bindings[j];
            setBinding.stageFlags = stage;
            setBinding.binding = binding.binding;
            setBinding.descriptorType = (VkDescriptorType)binding.descriptor_type;
            setBinding.pImmutableSamplers = nullptr;
            setBinding.descriptorCount = 1;
            for (uint32_t dims = 0; dims < binding.array.dims_count; ++dims)
            {
                setBinding.descriptorCount *= binding.array.dims[dims];
            }
        }
    }

    uint32_t pushCount = 0;
    spvReflectEnumeratePushConstantBlocks(&spvModule, &pushCount, nullptr);
    std::vector<SpvReflectBlockVariable *> pushVars(pushCount);
    spvReflectEnumeratePushConstantBlocks(&spvModule, &pushCount, pushVars.data());

    if (shader->ranges.capacity() != 1)
        shader->ranges.resize(1);

    uint32_t pushVarOffset = 0;
    for (uint32_t i = 0; i < pushCount; ++i)
    {
        SpvReflectBlockVariable &var = *(pushVars[i]);

        VkPushConstantRange &range = shader->ranges[0];
        range.stageFlags |= stage;
        range.size = var.size;
        range.offset = pushVarOffset;

        pushVarOffset += range.size;
    }

    spvReflectDestroyShaderModule(&spvModule);

    return shaderModule;
}

void CreateShader(Shader *shader, const char *vertPath, const char *fragPath)
{
    if (!glslangInitialized)
    {
        glslang::InitializeProcess();
        glslangInitialized = true;
    }

    std::vector<char> vertCode;
    std::vector<char> fragCode;

    std::thread vertThread([&]()
    {
        vertCode = CompileToSpirv(vertPath);
    });

    std::thread fragThread([&]()
    {
        fragCode = CompileToSpirv(fragPath);
    });

    vertThread.join();
    fragThread.join();

    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.pNext = nullptr;
    shaderStage.flags = 0;
    shaderStage.pName = "main";
    shaderStage.pSpecializationInfo = nullptr;

    shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage.module = CreateShaderModule(shader, vertCode);
    shader->stages.push_back(shaderStage);

    VkPipelineShaderStageCreateInfo fragStage = shaderStage;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = CreateShaderModule(shader, fragCode);
    shader->stages.push_back(fragStage);
}

void DestroyShader(Shader *shader)
{
    for (int i = 0; i < shader->stages.size(); ++i)
    {
        vkDestroyShaderModule(renderer.device, shader->stages[i].module, nullptr);
    }
}
