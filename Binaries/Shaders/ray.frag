#version 430 core

in vec2 texCoord;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform float SkyFactor;

out vec4 outColor;


const vec3 upVec = vec3(0, 0, 1);

const float fov = 70.0f;
const float near = 0.1f;
const float far = 1000.0f;
const vec2 resolution = vec2(1280, 720);

const float ratio = resolution.x/resolution.y;

const float EPSILON = 0.0001f;

const vec3 lightPosition = vec3(0, 0, 10);

float smoothMinimum( float a, float b)
{
    float k = 0.9;
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.0-h);
}

/// Operations sur les formes
vec2 sum(vec2 a, vec2 b)
{
    if (a.x < b.x)
        return a;

    return b;
}

vec2 difference(vec2 a, vec2 b)
{
    if (a.x > -b.x)
        return a;

    return -b;
}

vec2 intersection(vec2 a, vec2 b)
{
    if (a.x > b.x)
        return a;

    return b;
}

vec2 blend(vec2 a, vec2 b)
{
    float sm = smoothMinimum(a.x, b.x);
    if (sm == a.x)
        return a;
    else if (sm == b.x)
        return b;
    else
        return vec2(sm, a.y);
}

vec3 modulo( vec3 p, vec3 c )
{
    return mod(p, c) - 0.5*c;
}

/// Formes
float plane(vec3 p, vec4 n)
{
	return abs(dot(p + n.w * n.xyz, n.xyz));
}

float sphere(vec3 p, float r)
{
	return length(p) - r;
}

float torus(vec3 p, vec2 r)
{
	vec2 q = vec2(length(p.xy)-r.x, p.z);

	return length(q) - r.y;
}

float box(vec3 p, vec3 s, float r)
{
    return length(max(abs(p) - s, 0.0f)) - r;
}

/// CIEL
const vec3 sunDir = normalize( vec3(.0, 1, 0.1) );

float turbidity = 10;

vec2 SunPos = vec2(0, 0.2);

const float mieCoefficient = 0.005;
const float mieDirectionalG = 0.80;


// constants for atmospheric scattering
const float e = 2.71828182845904523536028747135266249775724709369995957;
const float pi = 3.141592653589793238462643383279502884197169;

const float n = 1.0003; // refractive index of air
const float N = 2.545E25; // number of molecules per unit volume for air at
// 288.15K and 1013mb (sea level -45 celsius)

// wavelength of used primaries, according to preetham
const vec3 primaryWavelengths = vec3(680E-9, 550E-9, 450E-9);

// mie stuff
// K coefficient for the primaries
const vec3 K = vec3(0.686, 0.678, 0.666);
const float v = 4.0;

// optical length at zenith for molecules
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const vec3 up = vec3(0.0, 0.0, 1.0);

const float sunIntensity = 1000.0;

// Angular sun size - physical sun is 0.53 degrees
float sunSize = 0.53;

float sunAngularDiameterCos = cos(sunSize*pi/180.0);

// earth shadow hack
const float cutoffAngle = pi/1.95;
const float steepness = 1.5;

float RayleighPhase(float cosViewSunAngle)
{
	return (3.0 / (16.0*pi)) * (1.0 + pow(cosViewSunAngle, 2.0));
}

vec3 totalMie(vec3 primaryWavelengths, vec3 K, float T)
{
	float c = (0.2 * T ) * 10E-18;
	return 0.434 * c * pi * pow((2.0 * pi) / primaryWavelengths, vec3(v - 2.0)) * K;
}

float hgPhase(float cosViewSunAngle, float g)
{
	return (1.0 / (4.0*pi)) * ((1.0 - pow(g, 2.0)) / pow(1.0 - 2.0*g*cosViewSunAngle + pow(g, 2.0), 1.5));
}

float SunIntensity(float zenithAngleCos)
{
	return sunIntensity * max(0.0, 1.0 - exp(-((cutoffAngle - acos(zenithAngleCos))/steepness)));
}

vec3 fromSpherical(vec2 p)
{
	return vec3(
		cos(p.x)*sin(p.y),
		sin(p.x)*sin(p.y),
		cos(p.y));
}

vec3 sunDirection = normalize(fromSpherical((SunPos-vec2(0.0,0.5))*vec2(6.28,3.14)));

// Created by anatole duprat - XT95/2015
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//Just try to make a sky similar to http://www.scratchapixel.com/old/assets/Uploads/Atmospheric%20Scattering/as-aerial2.png in few lines
//Real sky here : http://www.scratchapixel.com/old/lessons/3d-advanced-lessons/simulating-the-colors-of-the-sky/atmospheric-scattering/

vec3 fastSky( vec3 rd )
{
    float zd = min(rd.z, 0.);
    rd.z = max(rd.z, 0.);

    vec3 col = vec3(0.);

    col += vec3(.4, .4 - exp( -rd.z*20. )*.3, .0) * exp(-rd.z*9.); // Red / Green
    col += vec3(.3, .5, .6) * (1. - exp(-rd.z*8.) ) * exp(-rd.z*.9) ; // Blue

    col = mix(col*1.2, vec3(.3),  1.-exp(zd*100.)); // Fog

    col += vec3(1.0, .8, .55) * pow( max(dot(rd,sunDir),0.), 15. ) * .6; // Sun
    col += pow(max(dot(rd, sunDir),0.), 150.0) *.15;

    return col;
}

vec3 sunsky(vec3 viewDir)
{

	// Cos Angles
	float cosViewSunAngle = dot(viewDir, sunDirection);
	float cosSunUpAngle = dot(sunDirection, up);
	float cosUpViewAngle = dot(up, viewDir);
		if (sunAngularDiameterCos == 1.0) {
	 return vec3(1.0,0.0,0.0);
}
	float sunE = SunIntensity(cosSunUpAngle);  // Get sun intensity based on how high in the sky it is
	// extinction (asorbtion + out scattering)
	// rayleigh coeficients
	vec3 rayleighAtX = vec3(5.176821E-6, 1.2785348E-5, 2.8530756E-5);

	// mie coefficients
	vec3 mieAtX = totalMie(primaryWavelengths, K, turbidity) * mieCoefficient;

	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = max(0.0, cosUpViewAngle);

	float rayleighOpticalLength = rayleighZenithLength / zenithAngle;
	float mieOpticalLength = mieZenithLength / zenithAngle;


	// combined extinction factor
	vec3 Fex = exp(-(rayleighAtX * rayleighOpticalLength + mieAtX * mieOpticalLength));

	// in scattering
	vec3 rayleighXtoEye = rayleighAtX * RayleighPhase(cosViewSunAngle);
	vec3 mieXtoEye = mieAtX *  hgPhase(cosViewSunAngle, mieDirectionalG);

	vec3 totalLightAtX = rayleighAtX + mieAtX;
	vec3 lightFromXtoEye = rayleighXtoEye + mieXtoEye;

	vec3 somethingElse = sunE * (lightFromXtoEye / totalLightAtX);

	vec3 sky = somethingElse * (1.0 - Fex);
	sky *= mix(vec3(1.0),pow(somethingElse * Fex,vec3(0.5)),clamp(pow(1.0-dot(up, sunDirection),5.0),0.0,1.0));
	// composition + solar disc

	float sundisk = smoothstep(sunAngularDiameterCos,sunAngularDiameterCos+0.00002,cosViewSunAngle);
	vec3 sun = (sunE * 19000.0 * Fex)*sundisk;

	return 0.01*(sun+sky);
}

vec3 sky(vec3 viewDir)
{
	// Cos Angles
	float cosViewSunAngle = dot(viewDir, sunDirection);
	float cosSunUpAngle = dot(sunDirection, up);
	float cosUpViewAngle = dot(up, viewDir);

	float sunE = SunIntensity(cosSunUpAngle);  // Get sun intensity based on how high in the sky it is
	// extinction (asorbtion + out scattering)
	// rayleigh coeficients
	vec3 rayleighAtX = vec3(5.176821E-6, 1.2785348E-5, 2.8530756E-5);

	// mie coefficients
	vec3 mieAtX = totalMie(primaryWavelengths, K, turbidity) * mieCoefficient;

	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = max(0.0, cosUpViewAngle);

	float rayleighOpticalLength = rayleighZenithLength / zenithAngle;
	float mieOpticalLength = mieZenithLength / zenithAngle;


	// combined extinction factor
	vec3 Fex = exp(-(rayleighAtX * rayleighOpticalLength + mieAtX * mieOpticalLength));

	// in scattering
	vec3 rayleighXtoEye = rayleighAtX * RayleighPhase(cosViewSunAngle);
	vec3 mieXtoEye = mieAtX *  hgPhase(cosViewSunAngle, mieDirectionalG);

	vec3 totalLightAtX = rayleighAtX + mieAtX;
	vec3 lightFromXtoEye = rayleighXtoEye + mieXtoEye;

	vec3 somethingElse = sunE * (lightFromXtoEye / totalLightAtX);

	vec3 sky = somethingElse * (1.0 - Fex);
	sky *= mix(vec3(1.0),pow(somethingElse * Fex,vec3(0.5)),clamp(pow(1.0-dot(up, sunDirection),5.0),0.0,1.0));
	// composition + solar disc

	float sundisk = smoothstep(sunAngularDiameterCos,sunAngularDiameterCos+0.00002,cosViewSunAngle);
	vec3 sun = (sunE * 19000.0 * Fex)*sundisk;

	return SkyFactor*0.01*(sky);
}

vec3 sun(vec3 viewDir)
{

	// Cos Angles
	float cosViewSunAngle = dot(viewDir, sunDirection);
	float cosSunUpAngle = dot(sunDirection, up);
	float cosUpViewAngle = dot(up, viewDir);

	float sunE = SunIntensity(cosSunUpAngle);  // Get sun intensity based on how high in the sky it is
	// extinction (asorbtion + out scattering)
	// rayleigh coeficients
	vec3 rayleighAtX = vec3(5.176821E-6, 1.2785348E-5, 2.8530756E-5);

	// mie coefficients
	vec3 mieAtX = totalMie(primaryWavelengths, K, turbidity) * mieCoefficient;

	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = max(0.0, cosUpViewAngle);

	float rayleighOpticalLength = rayleighZenithLength / zenithAngle;
	float mieOpticalLength = mieZenithLength / zenithAngle;


	// combined extinction factor
	vec3 Fex = exp(-(rayleighAtX * rayleighOpticalLength + mieAtX * mieOpticalLength));

	// in scattering
	vec3 rayleighXtoEye = rayleighAtX * RayleighPhase(cosViewSunAngle);
	vec3 mieXtoEye = mieAtX *  hgPhase(cosViewSunAngle, mieDirectionalG);

	vec3 totalLightAtX = rayleighAtX + mieAtX;
	vec3 lightFromXtoEye = rayleighXtoEye + mieXtoEye;

	vec3 somethingElse = sunE * (lightFromXtoEye / totalLightAtX);

	vec3 sky = somethingElse * (1.0 - Fex);
	sky *= mix(vec3(1.0),pow(somethingElse * Fex,vec3(0.5)),clamp(pow(1.0-dot(up, sunDirection),5.0),0.0,1.0));
	// composition + solar disc

//	float sundisk = smoothstep(sunAngularDiameterCos,sunAngularDiameterCos+0.00002,cosViewSunAngle);
	float sundisk =
		sunAngularDiameterCos < cosViewSunAngle ? 1.0 : 0.0;
	//	smoothstep(sunAngularDiameterCos,sunAngularDiameterCos+0.00002,cosViewSunAngle);
	vec3 sun = (sunE * 19000.0 * Fex)*sundisk;

	return 0.01*sun;
}
/// CIEL


vec2 getClosestObject(vec3 p)
{
    vec2 sol = vec2(plane(p, vec4(normalize(vec3(0, 0, 1)), 10)), 0);
    vec2 cubes = vec2(box(modulo(p, vec3(11, 11, 11)), vec3(2, 2, 2), 0.2), 1);
    vec2 cube = vec2(box(p - vec3(2, 2, -2), vec3(2, 2, 2), 0.2), 1);
    vec2 tores = vec2(torus(modulo(p, vec3(16, 16 ,16)), vec2(3, 1)), 1);
    vec2 lumiere = vec2(sphere(p-lightPosition, 0.5), 2);

//	return sum( sum(sol, intersection(cubes, tores)),  lumiere);
	return sum(sol, sum(lumiere, cube));
}

/// fonctions de couleur
vec3 floorColor(vec3 p)
{
	if (fract(p.x*0.2)>0.2)
	{
		if (fract(p.y*0.2)>0.2)
			return vec3(0,0.1,0.2);
		else
			return vec3(1,1,1);
	}
	else
	{
		if (fract(p.y*.2)>.2)
			return vec3(1,1,1);
		else
			return vec3(0.3,0,0);
	}
}

vec3 getNormal(vec3 pos, float minDistance)
{
    float eps = 0.02;
    vec3 n;
    n.x = getClosestObject( vec3(pos.x+eps, pos.y, pos.z) ).x - minDistance;
    n.y = getClosestObject( vec3(pos.x, pos.y+eps, pos.z) ).x - minDistance;
    n.z = getClosestObject( vec3(pos.x, pos.y, pos.z+eps) ).x - minDistance;
    return normalize(n);
}

float shadow(vec3 ro, vec3 rd)
{
    vec2 minDistance;
    vec3 rayPosition = ro + 5.0f*EPSILON*rd;

    for (int steps = 0 ; steps < 512 ; steps++)
	{
		minDistance = getClosestObject(rayPosition);
		rayPosition += minDistance.x * rd;

		if (minDistance.y == 2)
            return 1.0f;

		if (abs(minDistance.x) < EPSILON)
			return 0.2f;

	}

    return 1.0f;
}


float softShadow(vec3 ro, vec3 rd, float mint, float maxt)
{
    float res = 1.0f;
    float k = 8.0f;
    for( float t=mint; t < maxt; )
    {
        float h = getClosestObject(ro + rd*t).x;
        if( h < EPSILON )
            return 0.2f;

        res = min( res, k*h/t );
        t += h;
    }

    return clamp(res, 0.2f, 1.0f);
}

void main()
{
	vec2 pixelPos = 2.0f * texCoord -1.0f;

	/// Determination du rayon
	vec3 front = (cameraPosition+cameraDirection);


	vec3 u = normalize(cross(upVec, cameraDirection)); // right
	vec3 v = cross(cameraDirection, u); // up

	vec3 scrCoord = cameraDirection - pixelPos.x * u * ratio + pixelPos.y * v;
	vec3 rayDirection = normalize(scrCoord);


	/// Raymarching
	vec3 rayPosition = cameraPosition + near * rayDirection;
	vec2 minDistance;
	float distanceTotale = near;

	int steps = 0, maxSteps = 256;
	while (++steps <= maxSteps)
	{
		minDistance = getClosestObject(rayPosition);
		rayPosition += minDistance.x * rayDirection;

		distanceTotale += minDistance.x;

        if (steps == maxSteps || distanceTotale >= far-near)
        {
            outColor = vec4(sunsky(rayDirection), 1.0f);
            return;
        }

		if (abs(minDistance.x) < EPSILON)
			break;

	}


	/// Calcul de la couleur du pixel
	vec3 couleur;
    if (minDistance.y == 0)
        couleur = floorColor(rayPosition);
    else if (minDistance.y == 1)
        couleur = vec3(0.6,0.6,0.8);
    else if (minDistance.y == 2)
        couleur = vec3(10, 10, 10);

	/// Calcul de l'eclairage du pixel
    vec3 ambient = vec3(0.3f, 0.3f, 0.3f);
    vec3 diffuse = vec3(0.8f, 0.8f, 0.8f);
    vec3 specular = vec3(0.2f, 0.2f, 0.2f);
    float exponent = 256.0f;


    vec3 diffuseColor = vec3(1, 1, 1);

	float lightDistance = length(lightPosition - rayPosition);
    vec3 lightDir = (lightPosition - rayPosition) / lightDistance;

	vec3 norm = getNormal(rayPosition, minDistance.x);
	vec3 viewDir = normalize(cameraPosition - rayPosition);


//	float inLight = shadow(rayPosition, -vec3(-1, -1, -1));
	float inLight = softShadow(rayPosition, -vec3(-1, -1, -1), 10.0f*EPSILON, 500.0f);


    //ambient
		vec3 ambientResult = ambient * diffuseColor * couleur;

    //diffuse
		float diffuseCoefficient = max(0.0f, dot(norm, lightDir));
		vec3 diffuseResult = diffuse * diffuseCoefficient * diffuseColor * couleur;

    //specular
		float specularCoefficient = 0.0f;
		if(diffuseCoefficient != 0.0f)
			specularCoefficient = pow(max(dot(viewDir, reflect(-lightDir, norm)), 0.0f), exponent);
		vec3 specularResult = specular * specularCoefficient * diffuseColor;

    //attenuation
		float attenuation = 1.0f / (1 + 0.01 * lightDistance);

    outColor = vec4(inLight * (ambientResult + attenuation*(diffuseResult + specularResult)), 1.0f);
}
