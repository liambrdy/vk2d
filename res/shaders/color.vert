#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vertPos;

layout (binding = 0, set = 0) uniform Camera
{
    mat4 projection;
};

layout (push_constant) uniform PushConstant
{
    mat4 model;
    vec4 texCoord;
    vec4 color;
};

void main()
{
    gl_Position = projection * model * vec4(vertPos, 0.0, 1.0);
}