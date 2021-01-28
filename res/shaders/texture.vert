#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vertPos;

layout (location = 0) out vec2 fragUv;

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

    vec2 texPos = vec2(texCoord.x, texCoord.y);
    vec2 texSize = vec2(texCoord.z, texCoord.w);
    vec2 uvs[6] = vec2[](
        texPos, vec2(texPos.x + texSize.x, texPos.y), texPos + texSize,
        texPos + texSize, vec2(texPos.x, texPos.y + texSize.y), texPos);
    fragUv = uvs[gl_VertexIndex];
}