#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#include "lib/brdf.glsl"

float getSquareFalloffAttenuation(float distanceSquare, float falloff)
{
	float factor = distanceSquare * falloff;
	float smoothFactor = saturate(1.0 - factor * factor);
	// We would normally divide by the square distance here
	// but we do it at the call site
	return smoothFactor * smoothFactor;
}

float getDistanceAttenuation(vec3 posToLight, float falloff)
{
	float distanceSquare = dot(posToLight, posToLight);
	float attenuation = getSquareFalloffAttenuation(distanceSquare, falloff);
	// Assume a punctual light occupies a volume of 1cm to avoid a division by 0
	return attenuation * 1.0 / max(distanceSquare, 1e-4);
}


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


void AccumulatePointLight(int i, vec3 V, BRDFData data, inout SplitLighting lighting)
{
	const Light light = lights[i];
	vec3 L = light.position - data.position;
	float attenuation = getDistanceAttenuation(L, light.falloff);
	if (attenuation > 0.0 && dot(data.normal, L) > 0.0)
	{
		float shadow = 1.0;
		attenuation = light.intensity * attenuation * shadow;
		vec3 color = light.color * attenuation;

		SplitLighting radiance = BRDF(data, V, normalize(L));

		lighting.diffuse += radiance.diffuse * color;
		lighting.specular += radiance.specular * color;
	}
}

#endif
