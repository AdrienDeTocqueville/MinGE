#version 430 core

in vec3 vertex;
in vec3 normal;
in vec2 texCoord;

uniform sampler2D texture;

out vec4 outColor;

void main()
{
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
const vec3 lightPos = vec3(0, 0, 30.0f);

    /// Texture
		vec4 textureResult = texture(texture, texCoord);

    /// Ambient
		vec3 ambient = 0.5f * lightColor;

    /// Diffuse
		vec3 norm = normalize(normal);
		vec3 lightDir = normalize(lightPos - vertex);
		vec3 diffuse = max(dot(norm, lightDir), 0.0f) * lightColor;

    /// Specular
	//    float specularStrength = 0.5f;
	//    vec3 viewDir = normalize(viewPos - vertex);
	//    vec3 reflectDir = reflect(-lightDir, norm);
	//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	//    vec3 specular = specularStrength * spec * lightColor;

    vec4 lightResult = vec4(ambient + diffuse, 1.0f);

    /// Final Color
		outColor = lightResult * textureResult;
}
