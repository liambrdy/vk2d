#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

layout (binding = 0, set = 1) uniform sampler2D tex;

layout (push_constant) uniform PushConstant
{
    mat4 model;
    vec4 texCoord;
    vec4 color;
};

void main()
{
    outColor = texture(tex, fragUv) * color;
}