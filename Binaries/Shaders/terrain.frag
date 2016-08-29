#version 430 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform int  side;
uniform vec4 scale;

uniform sampler2D colorMap;

uniform sampler2D detailMap;
uniform sampler2D detailTex1;
uniform sampler2D detailTex2;
uniform sampler2D detailTex3;
uniform sampler2D detailTex4;

out vec4 outColor;

const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
const vec3 lightPos = vec3(0, 0, 30.0f);

void main()
{
	/// Texture
		vec2 tileCoord = texCoord * side;

		vec4 color = texture(colorMap, texCoord);

		vec4 alpha = texture(detailMap, texCoord);
		vec4 textureResult = texture(detailTex1, tileCoord/scale.x) * alpha.x
                           + texture(detailTex2, tileCoord/scale.y) * alpha.y
                           + texture(detailTex3, tileCoord/scale.z) * alpha.z
                           + texture(detailTex4, tileCoord/scale.w) * alpha.w;

    ///There are 3 Color Modes in EarthSculptor
//		textureResult += color - 0.5;
//		textureResult = textureResult + color;
		textureResult = textureResult * color;

	/// Ambient
		vec3 ambient = 0.8f * lightColor;

	/// Diffuse
		vec3 norm = normalize(normal);
		vec3 lightDir = normalize(lightPos - fragPos);
		vec3 diffuse = max(dot(norm, lightDir), 0.0f) * lightColor;

    /// Specular
	//    float specularStrength = 0.5f;
	//    vec3 viewDir = normalize(viewPos - fragPos);
	//    vec3 reflectDir = reflect(-lightDir, norm);
	//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	//    vec3 specular = specularStrength * spec * lightColor;

    vec4 lightResult = vec4(ambient + diffuse, 1.0f);
	
    /// Final Color
		outColor = lightResult * textureResult;
}
