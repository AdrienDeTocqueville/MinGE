#include "lib/builtin.glsl"
#include "lib/light.glsl"
#include "lib/brdf.glsl"

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



float compute_shadow()
{
#ifdef RECEIVE_SHADOW
	// TODO: shadow map atlas
	vec3 coords = in_fs.pos_light_space.xyz / in_fs.pos_light_space.w;
	coords = coords * 0.5f + 0.5f;

	return texture(SHADOW_MAP, coords.xy).r < coords.z ? 0.5 : 1.0;
#else
	return 1.0f;
#endif
}


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

        vec3 diffuseColor = (1.0 - metallic) * color.rgb;
        vec3 f0 = 0.04 * (1.0 - metallic) + color.rgb * metallic;
        float linearRoughness = roughness * roughness;

	vec3 N = normalize(in_fs.normal);
	vec3 V = normalize(camera.position - in_fs.pos);

	// Evaluate punctual lights
	vec3 Lo = vec3(0.0f);
	for (int i = 0; i < lights.length(); i++)
	{
		vec3 L = lights[i].position - in_fs.pos;
		vec3 radiance = BRDF(N, V, normalize(L), diffuseColor, f0, linearRoughness);
		float attenuation = getDistanceAttenuation(L, lights[i].falloff);
		Lo += (radiance * lights[i].color * lights[i].intensity)
			* attenuation * compute_shadow();
	}

	out_color = vec4(Lo, 1.0);
}
