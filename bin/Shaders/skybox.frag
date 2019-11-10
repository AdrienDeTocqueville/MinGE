#version 150 core

in vec3 point;

out vec4 outColor;

vec3 bezier(float t);

uniform vec3 c0;
uniform vec3 c1;

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
