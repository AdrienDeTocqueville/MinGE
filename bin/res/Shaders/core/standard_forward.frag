#include "lib/builtin.glsl"

BUILTIN_GLOBAL;

// In-out
in VS_FS {
	vec3 pos;
	vec3 normal;
	vec2 uv;
#ifdef RECEIVE_SHADOW
	vec4 pos_light_space;
#endif
} in_fs;

out vec4 out_color;

#include "lib/light.glsl"

// Material
#ifdef COLOR_MAP
uniform sampler2D color_map;
#else
uniform vec3 color = vec3(0.8f);
#endif

#ifdef METALLIC_MAP
uniform sampler2D metallic_map;
#else
uniform float metallic = 0.0f;
#endif

#ifdef ROUGHNESS_MAP
uniform sampler2D roughness_map;
#else
uniform float roughness = 0.5f;
#endif


void main()
{
#ifdef COLOR_MAP
	vec3 color = texture(color_map, in_fs.uv).xyz;
#endif
#ifdef METALLIC_MAP
	float metallic = texture(metallic_map, in_fs.uv).x;
#endif
#ifdef ROUGHNESS_MAP
	float roughness = texture(roughness_map, in_fs.uv).x;
#endif

	BRDFData brdf_data;
	brdf_data.position = in_fs.pos;
	brdf_data.normal = normalize(in_fs.normal);
	brdf_data.color = (1.0 - metallic) * color.rgb;
	brdf_data.f0 = 0.04 * (1.0 - metallic) + color.rgb * metallic;
	brdf_data.roughness = roughness * roughness;

	vec3 V = normalize(camera.position - in_fs.pos);
	SplitLighting lighting;
	lighting.diffuse = vec3(0.0f);
	lighting.specular = vec3(0.0f);

	// Evaluate punctual lights
	for (int i = 0; i < lights.length(); i++)
		AccumulatePointLight(i, V, brdf_data, lighting);

	vec3 final = brdf_data.color * lighting.diffuse + lighting.specular;
	out_color = vec4(final, 1.0);
}
