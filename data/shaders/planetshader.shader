#version 130

#define NUM_OCTAVES 5

in vec4 vertPos;
in vec4 color;

uniform vec2 size;
uniform float randSeed;
uniform bool gasGiant;
uniform bool frozen;
uniform bool atmosphere;
uniform bool atmosSameColor;
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

mat2 rotate(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	return mat2(c, -s, s, c);
}

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	vec2 worldPos = vertPos.xy - size;
	float radius = distance(worldPos, pixel) * 2.0;
	vec2 noisePos;
	float r = sqrt(dot(worldPos, worldPos)) / size.x;
	float f = (1.0 - sqrt(1.0 - r)) / (r);
	float light = dot(worldPos / 250.0, sunVec);
	float alpha = 1.0;
	vec3 cloudColor = vec3(1.0);

	if (atmosSameColor) cloudColor = color.rgb;
	
	if (!gasGiant) {
		noisePos = worldPos * f / 75.0;
	}
	else {
		noisePos = worldPos * f  / 300.0 * rotate(time / 3.0);
		
		/*vec2 angleVector = random2(vec2(randSeed));

		noisePos.x += angleVector.x * time;
		noisePos.y += angleVector.y * time;*/
	}

	float noiseVal = fbm(noisePos * 0.5 + 0.5) * 2.0;
	vec3 noiseVec = vec3(vec3(noiseVal) + 0.5 * 2.0);
	float cloudNoise = fbm((worldPos * f * 2.0 * rotate(time / 16.0) / 300.0f) + 1000.0) + 0.5;

	vec3 col = vec3(0.0);

	if (r < 0.9) {
		if (max(0.0f, noiseVal + 0.75f) < water && !gasGiant) {
				if (frozen) col = vec3(1.0, 1.0, 1.0);
				else col = vec3(0.0, 0.0, 1.0);
		}
		else {
			col = vec3(color.r, color.g, color.b) * noiseVec;
			if (gasGiant) {
				col += cloudNoise;
				col *= smoothstep(0.9, 0.8, r);
				alpha = smoothstep(0.9, 0.8, r);
			}
		}
		if (atmosphere) col += smoothstep(0.55, 0.6, cloudNoise) * cloudColor;
	}
	else {
		if (atmosphere) {
			vec3 atmosColor;

			if (atmosSameColor) atmosColor = color.rgb;
			else atmosColor = vec3(0.5, 0.8, 1.0);

			col = atmosColor * smoothstep(1.0, 0.9, r);
			alpha = smoothstep(1.0, 0.9, r);
		}
		else {
			col = vec3(0.0);
			alpha = 0.0;
		}
	}

	light /= size.x / 250.0;
	col *= light;
	col += light / 2.0;

	gl_FragColor = vec4(col, alpha);
}