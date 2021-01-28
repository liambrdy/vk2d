#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstant
{
    mat4 model;
    vec4 texCoord;
    vec4 color;
};

void main()
{
    outColor = color;
}