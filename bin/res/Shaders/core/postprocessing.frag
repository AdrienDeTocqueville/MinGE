in vec2 uv;
in vec4 gl_FragCoord;

out vec4 out_color;
out float gl_FragDepth;


uniform int hdr;

uniform sampler2D depth_buffer;
uniform sampler2D color_buffer;


vec3 ACES_Filmic(const vec3 x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

void main()
{
	const float gamma = 2.2;

	float depth = texture(depth_buffer, uv).r;
	gl_FragDepth = depth;

	vec3 color = texture(color_buffer, uv).rgb;
	if (hdr == 1)
		color = ACES_Filmic(color);

	// Gamma correction
	color = pow(color, vec3(1.0 / gamma));
	out_color = vec4(color, 1.0);
};
