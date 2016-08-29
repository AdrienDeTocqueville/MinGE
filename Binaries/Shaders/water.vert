#version 430 core

layout(location = 0) in vec3 in_Vertex;

uniform mat4 MVPMatrix;

out vec4 vertex;
out vec2 texCoord;

void main()
{
	vertex = MVPMatrix * vec4(in_Vertex, 1.0f);
	texCoord = in_Vertex.xy;

    gl_Position = vertex;
}
