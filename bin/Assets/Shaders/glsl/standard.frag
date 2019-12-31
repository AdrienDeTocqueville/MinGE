in VS_FS {
	vec3 fragPos;
	vec3 normal;
	vec2 uv;
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

const float PI = 3.14159265359;

out vec4 out_color;

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

	vec3 albedo = pow(color, vec3(2.2f));
	float ao        = 1.0f;

	vec3 N = normalize(in_fs.normal);
	vec3 V = normalize(VIEW_POS - in_fs.fragPos);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0f);
	{
		//vec3 L = normalize(lightPosition - in_fs.fragPos); // Point
		vec3 L = lightPosition; // Directional
		vec3 H = normalize(V + L);

		// Compute radiance
		//float attenuation = 1.0f / (distance * distance);
		float attenuation = 1.0f;
		vec3 radiance = lightColor * attenuation;

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
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}


	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 final = ambient + Lo;

	// HDR tonemapping
	final = final / (final + vec3(1.0));
	// gamma correct
	final = pow(final, vec3(1.0/2.2));

	out_color = vec4(final, 1.0);
}
