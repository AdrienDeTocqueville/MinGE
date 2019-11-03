#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Color;

uniform mat4 MVPMatrix;

out vec3 color;

void main()
{
	color = in_Color;
	gl_Position = MVPMatrix * vec4(in_Vertex, 1.0f);
}
