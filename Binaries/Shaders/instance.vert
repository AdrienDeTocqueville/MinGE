#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in mat4 in_Model;

uniform mat4 VPMatrix;
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
    mat4 model = in_Model * modelMatrix;

    vertex = vec3(model * vec4(in_Vertex, 1.0f));
    normal = mat3(transpose(inverse(model))) * in_Normal;
    texCoord = in_TexCoord;
    posFromLight1 = lightVPMatrix1 * vec4(vertex, 1.0);
    posFromLight2 = lightVPMatrix2 * vec4(vertex, 1.0);

    gl_Position = VPMatrix * vec4(vertex, 1.0f);
	gl_ClipDistance[0] = (dot(vec4(vertex, 1.0f), clipPlane));
}
