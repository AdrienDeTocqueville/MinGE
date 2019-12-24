layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

out VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 uv;
} out_vs;

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
	out_vs.fragPos = vec3(pos);
	out_vs.normal = mat3(model) * in_normal;
	out_vs.uv = in_uv;
#endif

	gl_Position = MATRIX_VP * pos;
}
