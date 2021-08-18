#version 130

#define NUM_OCTAVES 5

in vec4 vertPos;
in vec4 color;

uniform vec2 size;
uniform float randSeed;
uniform bool gasGiant;
uniform bool frozen;
uniform float water;
uniform float time;
uniform vec2 sunVec;

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

float fbm(vec2 x) {
	float v = 0.0;
	float a = 0.5;
	vec2 shift = vec2(100);
	// Rotate to reduce axial bias
	mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	vec2 worldPos = vertPos.xy - size;
	float radius = distance(worldPos, pixel) * 2.0;
	vec2 noisePos;
	float r = sqrt(dot(worldPos, worldPos)) / size.x;
	float f = (1.0 - sqrt(1.0 - r)) / (r);
	float light = dot(worldPos / 250.0, sunVec);
	
	if (!gasGiant) {
		noisePos = worldPos * f / 75.0;
	}
	else {
		noisePos = worldPos * f / 300.0;
		
		vec2 angleVector = random2(vec2(randSeed));

		noisePos.x += angleVector.x * time;
		noisePos.y += angleVector.y * time;
	}

	float noiseVal = fbm(noisePos * 0.5 + 0.5) * 2.0;
	vec3 noiseVec = vec3(vec3(noiseVal) + 0.5 * 2.0);

	if (r < 1.0) {
		if (max(0.0f, noiseVal + 0.75f) < water && !gasGiant) {
				if (frozen) gl_FragColor = vec4(vec3(1.0, 1.0, 1.0) * light, 1.0);
				else gl_FragColor = vec4(vec3(0.0, 0.0, 1.0) * light, 1.0);
		}
		else {
			gl_FragColor = vec4(vec3(color.r, color.g, color.b) * noiseVec * light, 1.0);

		}
	}
	else {
		gl_FragColor = vec4(0.0);
	}
}