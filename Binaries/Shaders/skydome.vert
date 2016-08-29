#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normals;
layout(location = 2) in vec2 in_Coords;

uniform mat4 MVPMatrix;

out vec3 point;


void main()
{
    gl_Position = MVPMatrix * vec4(in_Vertex, 1.0);

    point = in_Vertex;
}
