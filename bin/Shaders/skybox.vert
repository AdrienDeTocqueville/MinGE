#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normals;
layout(location = 2) in vec2 in_Coords;

uniform mat4 MATRIX_VP;
uniform vec3 cameraPosition;

out vec3 point;

void main()
{
    vec4 pos = MATRIX_VP * vec4(in_Position + cameraPosition, 1.0);
    gl_Position = pos.xyww;

    point = in_Position;
}
