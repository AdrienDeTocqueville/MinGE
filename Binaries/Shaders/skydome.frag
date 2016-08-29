#version 150 core

in vec3 point;

out vec4 outColor;

vec3 bezier(float t);

const vec3 c0 = vec3(0.67f, 0.92f, 1.0f);
const vec3 c1 = vec3(0.208608f, 0.348608f, 0.478608f);

void main()
{
	if (point.z < 0.0f)
		discard;
		
	float height = normalize(point).z;
					
	outColor = vec4(bezier(height), 1.0f);
}

float f(float x)
{
    return 0.47f * exp(-4*x);
}

vec3 bezier(float t)
{
	vec3 t0 = vec3(0, 0, 1);
	vec3 t1 = vec3(0.5, 0.5, 0);
    return (2*t*t*t - 3*t*t + 1)*c0 + (t*t*t - 2*t*t + t)*t0 + (-2*t*t*t + 3*t*t)*c1 + (t*t*t - t*t)*t1;
}