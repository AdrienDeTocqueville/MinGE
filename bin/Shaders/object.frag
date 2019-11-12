#version 430 core

in VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} fs_in;

// Camera
uniform vec3 cameraPosition;

// Light
uniform vec3 lightPosition;
uniform vec3 lightColor;

// Material
uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;

const float PI = 3.14159265359;

out vec4 outColor;

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
	vec3 albedo     = pow(texture(albedoMap, fs_in.texCoords).rgb, vec3(2.2));
	float metallic  = texture(metallicMap, fs_in.texCoords).r;
	float roughness  = texture(roughnessMap, fs_in.texCoords).r;
	float ao        = 1.0f;

	vec3 N = normalize(fs_in.normal);
	vec3 V = normalize(cameraPosition - fs_in.fragPos);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0f);
	{
		//vec3 L = normalize(lightPosition - fs_in.fragPos); // Point
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
		float denominator = max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
		vec3 specular = nominator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}


	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2));

	outColor = vec4(color, 1.0);
}
