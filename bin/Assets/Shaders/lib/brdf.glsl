#ifndef BRDF_GLSL
#define BRDF_GLSL

#include "lib/math.glsl"

float D_GGX(float linearRoughness, float NoH) {
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
    float a = NoH * linearRoughness;
    float k = linearRoughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelated(float linearRoughness, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = linearRoughness * linearRoughness;
    float GGXV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float GGXL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(const vec3 f0, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (vec3(1.0) - f0) * pow5(1.0 - VoH);
}

float Fd_Lambert() {
    return 1.0 / PI;
}

vec3 BRDF(vec3 n, vec3 v, vec3 l, vec3 diffuseColor, vec3 f0, float linearRoughness)
{
	vec3 h = normalize(v + l);

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoL = saturate(dot(n, l));
	float NoH = saturate(dot(n, h));
	float LoH = saturate(dot(l, h));

        // specular BRDF
        float D = D_GGX(linearRoughness, NoH);
        float V = V_SmithGGXCorrelated(linearRoughness, NoV, NoL);
        vec3  F = F_Schlick(f0, LoH);
        vec3 Fr = (D * V) * F;

        // diffuse BRDF
        vec3 Fd = diffuseColor * Fd_Lambert();

        return (Fd + Fr) * NoL;
}

#endif
