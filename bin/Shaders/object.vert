#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;

// Camera
uniform mat4 MATRIX_VP;

// Model
uniform mat4 MATRIX_M;
uniform mat4 MATRIX_N;

out VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

void main()
{
	vec4 pos = MATRIX_M * vec4(in_Position, 1.0f);

	vs_out.fragPos = vec3(pos);
	vs_out.normal = mat3(MATRIX_N) * in_Normal;
	vs_out.texCoords = in_TexCoords;

	gl_Position = MATRIX_VP * pos;
}
