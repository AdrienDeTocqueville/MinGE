layout(location = 0) in vec3 in_position;

uniform vec4 viewport;
uniform vec2 uv_dim;

out vec2 uv;

void main()
{
	vec2 in_pos = in_position.xy + vec2(0.5f);
	vec2 pos = viewport.xy + in_pos * viewport.zw;
	
	gl_Position = vec4(2.0f * pos - vec2(1.0f), 0.0f, 1.0f);
	
	uv = vec2(in_pos.x, 1.0f - in_pos.y) * uv_dim;
}
