#include "glsl/brdf.glsl"


in VS_FS {
	vec3 pos;
	vec3 normal;
	vec2 uv;
#ifdef CAST_SHADOW
	vec4 pos_light_space;
#endif
} in_fs;

// Material
#ifdef COLOR_MAP
uniform sampler2D color_map;
#else
uniform vec3 color;
#endif

#ifdef METALLIC_MAP
uniform sampler2D metallic_map;
#else
uniform float metallic;
#endif

#ifdef ROUGHNESS_MAP
uniform sampler2D roughness_map;
#else
uniform float roughness;
#endif

out vec4 out_color;


#ifdef CAST_SHADOW
float compute_shadow()
{
	vec3 coords = in_fs.pos_light_space.xyz / in_fs.pos_light_space.w;
	coords = coords * 0.5f + 0.5f;

	return texture(SHADOW_MAP, coords.xy).r < coords.z ? 0.5 : 1.0;
}
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

	float ao = 1.0f;

	vec3 N = normalize(in_fs.normal);
	vec3 V = normalize(VIEW_POS - in_fs.pos);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, color, metallic);

	vec3 Lo = vec3(0.0f);
#ifndef NO_LIGHT
	{
		vec3 L = normalize(vec3(0.0f) - in_fs.pos); // Point
		//vec3 L = LIGHT_DIR; // Directional
		vec3 H = normalize(V + L);

		// Compute radiance
#ifdef CAST_SHADOW
		float attenuation = compute_shadow();
#else
		float attenuation = 1.0f;
#endif
		vec3 radiance = LIGHT_COLOR * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);
		vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3  nominator   = NDF * G * F;
		float denominator = 4.0f * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
		vec3 specular = nominator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * color / PI + specular) * radiance * NdotL;
	}
#endif

	vec3 ambient = vec3(0.03) * color * ao;
	vec3 final = ambient + Lo;

	out_color = vec4(color, 1.0);
}
