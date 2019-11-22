#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;
layout(location = 3) in uvec4 in_Bones;
layout(location = 4) in vec4 in_Weights;

// Camera
uniform mat4 MATRIX_VP;

// Model
uniform mat4 MATRIX_M;
uniform mat4 MATRIX_N;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];


out VS_FS {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

void main()
{
	mat4 MATRIX_B =
		bones[in_Bones.x] * in_Weights.x +
		bones[in_Bones.y] * in_Weights.y +
		bones[in_Bones.z] * in_Weights.z +
		bones[in_Bones.w] * in_Weights.w;

	vec4 pos = MATRIX_B * vec4(in_Position, 1.0f);

	vs_out.fragPos = vec3(pos);
	vs_out.normal = mat3(MATRIX_B) * in_Normal;
	vs_out.texCoords = in_TexCoords;

	gl_Position = MATRIX_VP * pos;
}
