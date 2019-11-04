#version 430 core

in VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} fs_in;

// Camera
uniform vec3 cameraPosition;

// Light
uniform vec3 lightPosition;
uniform vec3 diffuseColor;
uniform float ambientCoefficient;
uniform float aConstant;
uniform float aLinear;
uniform float aQuadratic;

// Material
uniform sampler2D mainTexture;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float exponent;

out vec4 outColor;

void main()
{
	float lightDistance = length(lightPosition - fs_in.fragPos);
	vec3 texColor = texture(mainTexture, fs_in.texCoord).xyz;

	vec3 norm = normalize(fs_in.normal);
	vec3 lightDir = (lightPosition - fs_in.fragPos) / lightDistance;
	vec3 viewDir = normalize(cameraPosition - fs_in.fragPos);

	//ambient
	vec3 ambientResult = ambient * ambientCoefficient * diffuseColor * texColor;

	//diffuse
	float diffuseCoefficient = max(0.0f, dot(norm, lightDir));
	vec3 diffuseResult = diffuse * diffuseCoefficient * diffuseColor * texColor;

	//specular
	float specularCoefficient = 0.0f;
	if(diffuseCoefficient != 0.0f)
		specularCoefficient = pow(max(dot(viewDir, reflect(-lightDir, norm)), 0.0f), exponent);
	vec3 specularResult = specular * specularCoefficient * diffuseColor;

	//attenuation
	float attenuation = 1.0f / (aConstant + aLinear * lightDistance + aQuadratic * lightDistance * lightDistance);

	outColor = vec4(ambientResult + attenuation*(diffuseResult + specularResult), 0.0f);
}
