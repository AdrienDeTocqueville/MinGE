#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;

uniform mat4 MATRIX_VP;

out vec3 color;

void main()
{
	color = in_Color;
	gl_Position = MATRIX_VP * vec4(in_Position, 1.0f);
}
