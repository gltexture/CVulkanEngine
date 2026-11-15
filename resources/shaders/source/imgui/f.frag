#version 450

layout (location = 0) in vec2 inTextCoords;
layout (location = 1) in vec4 inColor;

layout (set = 0, binding = 0) uniform sampler2D textureSampler;

layout (location = 0) out vec4 outFragColor;

void main()
{
    outFragColor = inColor * texture(textureSampler, inTextCoords);
}