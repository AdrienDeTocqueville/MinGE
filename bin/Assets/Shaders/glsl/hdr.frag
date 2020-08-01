in vec2 uv;
in vec4 gl_FragCoord;

out vec4 out_color;
out float gl_FragDepth;


uniform int hdr;
uniform float exposure;

uniform sampler2D depth_buffer;
uniform sampler2D color_buffer;


void main()
{
	const float gamma = 2.2;

	float depth = texture(depth_buffer, uv).r;
	gl_FragDepth = depth;

	vec3 hdr_color = texture(color_buffer, uv).rgb;
	if (hdr == 1)
	{
		// reinhard
		// hdr_color = hdr_color / (hdr_color + vec3(1.0));
		// exposure
		hdr_color = vec3(1.0) - exp(-hdr_color * exposure);
	}

	// Gamma correction
	vec3 result = pow(hdr_color, vec3(1.0 / gamma));
	out_color = vec4(result, 1.0);
};
