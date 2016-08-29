#version 430 core

in float height;

out vec4 outColor;


void main()
{
	outColor = vec4(height, 0.5f, 0.5f, 0.0f);
}
