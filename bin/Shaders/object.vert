#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

uniform mat4 MVPMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec4 clipPlane;

out VS_FS
{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} vs_out;

void main()
{
    vs_out.fragPos = vec3(modelMatrix * vec4(in_Vertex, 1.0f));
    vs_out.normal = normalMatrix * in_Normal;
    vs_out.texCoord = in_TexCoord;

    gl_Position = MVPMatrix * vec4(in_Vertex, 1.0f);
    gl_ClipDistance[0] = (dot(vec4(vs_out.fragPos, 1.0f), clipPlane));
}
