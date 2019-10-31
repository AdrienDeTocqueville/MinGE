#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 3) in mat4 in_Model;

uniform mat4 VPMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = VPMatrix * in_Model * modelMatrix * vec4(in_Vertex, 1.0f);
}
