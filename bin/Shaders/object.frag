#version 430 core

in VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} fs_in;

uniform vec3 cameraPosition;

// Light
layout (std140) uniform Light
{
	vec4 lightPosition;
	vec4 diffuseColor;
	float ambientCoefficient;
	
	float aConstant;
	float aLinear;
	float aQuadratic;
};

// Material
uniform sampler2D mainTexture;

layout (std140) uniform Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float exponent;
};

out vec4 outColor;

void main()
{
	float lightDistance = length(lightPosition.xyz - fs_in.fragPos);
	vec3 texColor = texture(mainTexture, fs_in.texCoord).xyz;

	vec3 norm = normalize(fs_in.normal);
	vec3 lightDir = (lightPosition.xyz - fs_in.fragPos) / lightDistance;
	vec3 viewDir = normalize(cameraPosition - fs_in.fragPos);

	//ambient
	vec3 ambientResult = ambient.xyz * ambientCoefficient * diffuseColor.xyz * texColor;

	//diffuse
	float diffuseCoefficient = max(0.0f, dot(norm, lightDir));
	vec3 diffuseResult = diffuse.xyz * diffuseCoefficient * diffuseColor.xyz * texColor;

	//specular
	float specularCoefficient = 0.0f;
	if(diffuseCoefficient != 0.0f)
		specularCoefficient = pow(max(dot(viewDir, reflect(-lightDir, norm)), 0.0f), exponent);
	vec3 specularResult = specular.xyz * specularCoefficient * diffuseColor.xyz;

	//attenuation
	float attenuation = 1.0f / (aConstant + aLinear * lightDistance + aQuadratic * lightDistance * lightDistance);

	outColor = vec4(ambientResult + attenuation*(diffuseResult + specularResult), 1.0f);
}
