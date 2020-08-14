#ifndef BUILTIN_GLSL
#define BUILTIN_GLSL

struct Camera
{
	mat4 view_proj;
	//vec4 resolution;
	vec3 position;
	float pad0;
};

struct Light
{
	vec3 position;
	float radius;
	vec3 color;
	float pad0;
};

#define BUILTIN_GLOBAL \
layout (std140) readonly restrict buffer GLOBAL \
{ \
	Camera camera; \
	Light lights[]; \
}

#define BUILTIN_PER_OBJECT \
layout (std140) uniform PER_OBJECT \
{ \
	mat4 model_matrix; \
	mat4 normal_matrix; \
}

//#define BUILTIN_PER_OBJECT \
//layout (std140) readonly restrict buffer PER_SKINNED_OBJECT \
//{ \
//	mat4 model_matrix; \
//	vec4 normal_matrix; \
//	mat4 bones[]; \
//}

#endif // BUILTIN_GLSL
