in VS_FS {
	vec3 skyColor, viewDir;
};

uniform float sunSize;
uniform float sunBloom;

out vec4 outColor;

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3(1.0/2.2));
}

void main()
{
	float size2 = sunSize * sunSize;

	vec3 lightDir = vec3(
		LIGHT_DIR.x,
		LIGHT_DIR.z,
		LIGHT_DIR.y
	);
	float dist = 2.0 * (1.0 - dot(normalize(viewDir), lightDir));
	float sun  = exp(-dist/ sunBloom / size2) + step(dist, size2);
	float sun2 = min(sun * sun, 1.0);
	vec3 color = skyColor + sun2;
	color = toGamma(color);

	gl_FragColor = vec4(color, 1.0);
}
