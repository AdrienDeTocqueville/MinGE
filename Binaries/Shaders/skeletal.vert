#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in ivec4 in_BoneIds;
layout(location = 4) in vec4 in_Weights;

uniform mat4 MVPMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec4 clipPlane;

uniform mat4 bones[100];

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;

void main()
{
    mat4 skin = bones[in_BoneIds.x] * in_Weights.x +
                bones[in_BoneIds.y] * in_Weights.y +
                bones[in_BoneIds.z] * in_Weights.z +
                bones[in_BoneIds.w] * in_Weights.w;

    vec4 vertex = skin * vec4(in_Vertex, 1.0f);

    fragPos = vec3(modelMatrix * vertex);
    normal = vec3(normalMatrix * transpose(inverse(skin)) * vec4(in_Normal, 1.0f));
    texCoord = in_TexCoord;

    gl_Position = MVPMatrix * vertex;
	gl_ClipDistance[0] = (dot(vec4(fragPos, 1.0f), clipPlane));
}
