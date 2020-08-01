layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

#include "glsl/builtin.glsl"

BUILTIN_GLOBAL;

out vec3 color;

void main()
{
	color = in_color;
	gl_Position = camera.view_proj * vec4(in_position, 1.0f);
}
