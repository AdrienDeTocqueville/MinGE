#include "lib/builtin.glsl"

BUILTIN_GLOBAL;

// In-out
in VS_FS {
	vec3 pos;
	vec3 normal;
	vec2 uv;
} in_fs;

out vec4 out_color;

uniform vec2 tiling;
uniform sampler2D color_map;

// https://learnopengl.com/
vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
	// diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	vec3 viewDir = normalize(camera.position - fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;

	// simple attenuation
	float attenuation = 1.0 / dot(lightPos - fragPos, lightPos - fragPos);
	return (diffuse + specular) * attenuation;
}

void main()
{
	vec3 color = texture(color_map, in_fs.uv * tiling).rgb;
	vec3 lighting = vec3(0.0);
	for (int i = 0; i < lights.length(); i++)
		lighting += BlinnPhong(normalize(in_fs.normal), in_fs.pos, lights[i].position, lights[i].color);
	color *= lighting;

	out_color = vec4(color, 1.0);
}
