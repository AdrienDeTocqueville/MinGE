layout(location = 0) in vec3 in_position;

out VS_FS {
	vec3 skyColor, viewDir;
};

uniform vec4 perezCoeff[5];
uniform vec4 skyLuminance;
uniform float exposition;


vec3 Perez(vec3 A,vec3 B,vec3 C,vec3 D, vec3 E,float costeta, float cosgamma)
{
	float _1_costeta = 1.0 / costeta;
	float cos2gamma = cosgamma * cosgamma;
	float gamma = acos(cosgamma);
	vec3 f = (vec3(1.0) + A * exp(B * _1_costeta))
		* (vec3(1.0) + C * exp(D * gamma) + E * cos2gamma);
	return f;
}
vec3 convertXYZ2RGB(vec3 _xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
	return rgb;
}

void main()
{
	gl_Position = vec4(in_position.xy, 1.0f, 1.0f);

	//vec4 pos = MATRIX_VP * vec4(in_position + cameraPosition, 1.0);

	mat4 inv_VP = inverse(MATRIX_VP);
	vec4 rayStart = inv_VP * vec4(vec3(in_position.xy, -1.0), 1.0);
	vec4 rayEnd = inv_VP * vec4(vec3(in_position.xy, 1.0), 1.0);

	float temp = rayStart.y;
	rayStart.y = rayStart.z;
	rayStart.z = temp;
	temp = rayEnd.y;
	rayEnd.y = rayEnd.z;
	rayEnd.z = temp;

	rayStart = rayStart / rayStart.w;
	rayEnd = rayEnd / rayEnd.w;

	viewDir = normalize(rayEnd.xyz - rayStart.xyz);
	viewDir.y = abs(viewDir.y);

	vec3 lightDir = vec3(
		lightPosition.x,
		lightPosition.z,
		lightPosition.y
	);
	vec3 skyDir = vec3(0.0, 1.0, 0.0);

	// Perez coefficients.
	vec3 A = perezCoeff[0].xyz;
	vec3 B = perezCoeff[1].xyz;
	vec3 C = perezCoeff[2].xyz;
	vec3 D = perezCoeff[3].xyz;
	vec3 E = perezCoeff[4].xyz;

	float costeta = max(dot(viewDir, skyDir), 0.001);
	float cosgamma = clamp(dot(viewDir, lightDir), -0.9999, 0.9999);
	float cosgammas = dot(skyDir, lightDir);

	vec3 P = Perez(A,B,C,D,E, costeta, cosgamma);
	vec3 P0 = Perez(A,B,C,D,E, 1.0, cosgammas);

	float ratio = 1.0f / (skyLuminance.x+skyLuminance.y+skyLuminance.z);
	skyColor = vec3(
		skyLuminance.x * ratio,
		skyLuminance.y * ratio,
		skyLuminance.y
	);

	vec3 Yp = skyColor * P / P0;

	skyColor = vec3(
		Yp.x * Yp.z / Yp.y,
		Yp.z,
		(1.0 - Yp.x- Yp.y)*Yp.z/Yp.y
	);

	skyColor = convertXYZ2RGB(skyColor * exposition);
}
