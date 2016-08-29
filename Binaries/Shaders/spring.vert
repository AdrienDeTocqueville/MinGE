#version 430 core

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normals;
layout(location = 2) in vec2 in_TexCoords;

uniform mat4 MVPMatrix;
uniform mat4 transformation;
uniform vec3 p1;
uniform vec3 t1;

out float height;

mat4 clerp(float _x);
vec3 bezier(float t);

void main()
{
	height = in_Vertex.z;
	
	// vec4 position = clerp(height) * vec4(in_Vertex, 1.0f);
	
	vec4 position = vec4(bezier(height), 1.0f);
	
    gl_Position = MVPMatrix * position;
}

mat4 clerp(float _x)
{
	float x = _x*_x;
	
	return (1.0f-x) * mat4(1.0f) + x * transformation;
}

vec3 bezier(float t)
{
	vec3 p0 = vec3(0.0f);
	
	vec3 t0 = vec3(0, 0, 1);
	
    return (2*t*t*t - 3*t*t + 1)*p0 + (t*t*t - 2*t*t + t)*t0 + (-2*t*t*t + 3*t*t)*p1 + (t*t*t - t*t)*t1;
}