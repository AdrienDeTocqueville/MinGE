#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec2 in_TexCoord;

out vec2 texCoord;

void main()
{
    texCoord = in_TexCoord;
    gl_Position = vec4(in_Vertex, 1.0f);
}
