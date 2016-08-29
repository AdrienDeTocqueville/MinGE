#version 430 core

in vec4 vertex;
in vec2 texCoord;

uniform float tiling;
uniform float distorsionFactor;
uniform float offset;
uniform sampler2D reflected;
uniform sampler2D refracted;
uniform sampler2D dudvMap;
//uniform sampler2D normalMap;

out vec4 outColor;

//const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
//const vec3 lightPos = vec3(0, 0, 30.0f);

void main()
{
	vec2 windowCoord = (vertex.xy/vertex.w)*0.5 + 0.5;
	vec2 realCoord1 = vec2(texCoord.x * tiling, texCoord.y * tiling + offset);
	vec2 realCoord2 = vec2(texCoord.x * tiling + offset, texCoord.y * tiling);

    vec2 realCoord = (realCoord1+realCoord2)*0.5;

	vec2 distorsion = (texture(dudvMap  , realCoord).rg  *2.0 - 1.0) * distorsionFactor;
////	vec3 normal     = (texture(normalMap, realCoord).rgb *2.0 - 1.0);
//
	vec2 refractCoord = distorsion + windowCoord;
	vec2 reflectCoord = distorsion + vec2(windowCoord.x, 1-windowCoord.y);

	vec4 refraction = texture(refracted, clamp(refractCoord, 0.001, 0.999));
	vec4 reflection = texture(reflected, clamp(reflectCoord, 0.001, 0.999));

    /// Final Color
    outColor = mix(refraction, reflection, 0.5f);
    outColor = mix(outColor, vec4(0, 0.3, 0.5, 1), 0.2);
}
