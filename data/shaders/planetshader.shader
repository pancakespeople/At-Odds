#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 size;
uniform float randSeed;
uniform bool gasGiant;
uniform bool frozen;
uniform float water;
uniform float time;

vec2 random2(vec2 st) {
	st = vec2(dot(st, vec2(127.1, 311.7)),
		dot(st, vec2(269.5, 183.3)));
	return -1.0 + 2.0 * fract(sin(st) * 43758.5453123 * randSeed);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st) {
	vec2 i = floor(st);
	vec2 f = fract(st);

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(mix(dot(random2(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
		dot(random2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
		mix(dot(random2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
			dot(random2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x), u.y);
}

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	vec2 worldPos = vertPos.xy - size;
	float radius = distance(worldPos, pixel) * 2.0;
	vec2 noisePos;
	
	if (!gasGiant) {
		noisePos = worldPos / 50.0;
	}
	else {
		noisePos = (worldPos * pow(radius, 0.4)) / 3000.0;
		
		vec2 angleVector = random2(vec2(randSeed)) * 5.0;

		noisePos.x += angleVector.x * time;
		noisePos.y += angleVector.y * time;
	}

	float noiseVal = noise(noisePos * 0.5 + 0.5);
	vec4 noiseVec = vec4(vec3(noiseVal) + 0.5 * 2.0, 1.0);

	if (max(0.0f, noiseVal + 0.5f) < water && !gasGiant) {
		if (frozen) gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		else gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else {
		gl_FragColor = color * noiseVec;
	}
}