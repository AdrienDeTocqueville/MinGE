in vec2 uv;

uniform sampler2D color_buffer;
uniform int hdr;
uniform float exposure;

out vec4 out_color;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(color_buffer, uv).rgb;
	if(hdr)
	{
		// reinhard
		// vec3 result = hdrColor / (hdrColor + vec3(1.0));
		// exposure
		vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
		// also gamma correct while we're at it
		result = pow(result, vec3(1.0 / gamma));
		out_color = vec4(result, 1.0);
	}
	else
	{
		vec3 result = pow(hdrColor, vec3(1.0 / gamma));
		out_color = vec4(result, 1.0);
	}
};
