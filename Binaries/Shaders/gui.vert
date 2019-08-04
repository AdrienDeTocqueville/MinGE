#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec2 in_TexCoord;

uniform mat4 modelMatrix;

out vec2 texCoord;

void main()
{
	texCoord = in_TexCoord;

	vec3 pos = (modelMatrix * vec4(in_Vertex, 1.0)).xyz;

    gl_Position = vec4(pos.xy, 0.0, 1.0);
}
