#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_TexCoord;

uniform mat4 MVPMatrix;
uniform mat4 modelMatrix;
uniform vec4 clipPlane;
uniform mat4 lightVPMatrix1;
uniform mat4 lightVPMatrix2;

out vec3 vertex;
out vec3 normal;
out vec2 texCoord;
out vec4 posFromLight1;
out vec4 posFromLight2;

void main()
{
    vertex = vec3(modelMatrix * vec4(in_Vertex, 1.0f));
    normal = mat3(transpose(inverse(modelMatrix))) * in_Normal;
    texCoord = in_TexCoord.xy;
    posFromLight1 = lightVPMatrix1 * vec4(vertex, 1.0);
    posFromLight2 = lightVPMatrix2 * vec4(vertex, 1.0);

    gl_Position = MVPMatrix * vec4(in_Vertex, 1.0f);
	gl_ClipDistance[0] = (dot(vec4(vertex, 1.0f), clipPlane));
}
