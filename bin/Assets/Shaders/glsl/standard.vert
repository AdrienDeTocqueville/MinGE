layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uvec4 in_bones;
layout(location = 4) in vec4 in_weights;

#ifdef FORWARD_PASS
out VS_FS
{
	vec3 pos;
	vec3 normal;
	vec2 uv;
#ifdef CAST_SHADOW
	vec4 pos_light_space;
#endif
} out_vs;
#endif

#ifdef SKINNED
const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];
#endif

void main()
{
#ifdef SKINNED
	mat4 model = MATRIX_M * (
		bones[in_bones.x] * in_weights.x +
		bones[in_bones.y] * in_weights.y +
		bones[in_bones.z] * in_weights.z +
		bones[in_bones.w] * in_weights.w);
#else
	mat4 model = MATRIX_M;
#endif

	vec4 pos = model * vec4(in_position, 1.0f);

#ifdef FORWARD_PASS
	out_vs.pos = vec3(pos);
	out_vs.normal = mat3(model) * in_normal;
	out_vs.uv = in_uv;
#ifdef CAST_SHADOW
	out_vs.pos_light_space = MATRIX_LIGHT * pos;
#endif
#endif

	gl_Position = MATRIX_VP * pos;
}
