#version 430 core

in vec2 texCoord;

uniform sampler2D texture;

out vec4 outColor;

void main()
{
	outColor = texture(texture, texCoord);
}
