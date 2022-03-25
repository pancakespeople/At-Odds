#version 130

in vec4 color;

uniform float randSeed;
uniform bool gasGiant;
uniform bool frozen;
uniform float water;

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
	for (int i = 0; i < 5; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

mat2 rotate(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	return mat2(c, -s, s, c);
}

void main() {
	vec2 uv = gl_TexCoord[0].xy - 0.5;
	float radius = length(uv) * 2.0;
	
	vec2 noisePos;

	if (!gasGiant) {
		noisePos = uv;
	}
	else {
		noisePos = uv;
	}

	float noiseVal = fbm(noisePos * 5.0 + 0.5) * 4.0;
	vec3 noiseVec = vec3(vec3(noiseVal) + 0.5 * 2.0);

	vec3 col = vec3(0.0);

	if (max(0.0f, noiseVal + 0.75f) < water && !gasGiant) {
		if (frozen) col = vec3(1.0, 1.0, 1.0);
		else col = vec3(0.0, 0.0, 1.0);
	}
	else {
		col = vec3(color.r, color.g, color.b) * noiseVec;
		/*if (gasGiant) {
			col *= smoothstep(0.9, 0.8, radius);
		}*/
	}

	gl_FragColor = vec4(col, 1.0);
}