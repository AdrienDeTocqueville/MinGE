#ifndef BRDF_GLSL
#define BRDF_GLSL

#include "lib/math.glsl"

struct BRDFData
{
	vec3 position;
	vec3 normal;

	vec3 color;
	vec3 f0;
	float roughness;
};
struct SplitLighting
{
	vec3 diffuse;
	vec3 specular;
};

float D_GGX(float linearRoughness, float NoH)
{
	// Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
	float a = NoH * linearRoughness;
	float k = linearRoughness / (1.0 - NoH * NoH + a * a);
	return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float linearRoughness, float NoV, float NoL)
{
	// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
	float a2 = linearRoughness * linearRoughness;
	float GGXV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
	float GGXL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
	return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(const vec3 f0, float VoH)
{
	// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
	return f0 + (vec3(1.0) - f0) * pow5(1.0 - VoH);
}

float Fd_Lambert()
{
	return 1.0 / PI;
}

SplitLighting BRDF(BRDFData data, vec3 v, vec3 l)
{
	vec3 h = normalize(v + l);
	vec3 n = data.normal;

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = saturate(dot(n, l));
	float NoH = saturate(dot(n, h));
	float LoH = saturate(dot(l, h));

	// specular BRDF
	float D = D_GGX(data.roughness, NoH);
	float V = V_SmithGGXCorrelated(data.roughness, NoV, NoL);
	vec3  F = F_Schlick(data.f0, LoH);
	vec3 Fs = (D * V) * F;

	// diffuse BRDF
	float Fd = Fd_Lambert();

	return SplitLighting(vec3(Fd * NoL), Fs * NoL);
}

#endif
