#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

// Camera
uniform mat4 MATRIX_VP;
uniform vec4 clipPlane;

// Model
uniform mat4 MATRIX_M;
uniform mat4 MATRIX_N;

out VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} vs_out;

void main()
{
	vec4 vertex = MATRIX_M * vec4(in_Vertex, 1.0f);
	vs_out.fragPos = vec3(vertex);
	vs_out.normal = mat3(MATRIX_N) * in_Normal;
	vs_out.texCoord = in_TexCoord;

	gl_Position = MATRIX_VP * vertex;
	gl_ClipDistance[0] = (dot(vec4(vs_out.fragPos, 1.0f), clipPlane));
}
