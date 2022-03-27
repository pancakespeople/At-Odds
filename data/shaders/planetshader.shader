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

vec3 getBumpNormal(vec2 uv, float f, float noiseScale, vec2 offset, float diff) {
	vec2 nuv = uv - offset;

	vec2 uv1 = vec2(nuv.x + diff, nuv.y);
	float r1 = length(uv1);
	float f1 = (1.0 - sqrt(1.0 - r1)) / r1;

	vec2 uv2 = vec2(nuv.x, nuv.y + diff);
	float r2 = length(uv2);
	float f2 = (1.0 - sqrt(1.0 - r2)) / r2;

	vec2 uv3 = vec2(nuv.x - diff, nuv.y);
	float r3 = length(uv3);
	float f3 = (1.0 - sqrt(1.0 - r3)) / r3;

	vec2 uv4 = vec2(nuv.x, nuv.y - diff);
	float r4 = length(uv4);
	float f4 = (1.0 - sqrt(1.0 - r4)) / r4;

	float heightX = fbm(uv1 * f1 * noiseScale);
	float heightY = fbm(uv2 * f2 * noiseScale);
	float heightX2 = fbm(uv3 * f3 * noiseScale);
	float heightY2 = fbm(uv4 * f4 * noiseScale);

	vec2 heightDiff = vec2(heightX - heightX2, heightY - heightY2);
	float z = sqrt(1.0 - pow(length(heightDiff), 2.0));

	return vec3(heightDiff + uv, z);
}

void main() {
	vec2 uv = (gl_TexCoord[0].xy - 0.5) * 2.0;
	vec2 noisePos;
	float r = length(uv);
	float f = (1.0 - sqrt(1.0 - r)) / r;
	vec3 normal = vec3(uv, 0.0);
	float alpha = 1.0;
	float bumpRand = random2(vec2(randSeed, randSeed)).x * 0.005 + 0.005;
	vec3 cloudColor = vec3(1.0);

	if (atmosSameColor) cloudColor = color.rgb;
	
	if (!gasGiant) {
		noisePos = uv * f;
	}
	else {
		noisePos = uv * f * rotate(time / 3.0);
	}

	float noiseVal = fbm(noisePos * 2.0 + 0.5) * 4.0;
	vec3 noiseVec = vec3(vec3(noiseVal) + 0.5 * 2.0);
	float cloudNoise = fbm((uv * f * 2.0 * rotate(time / 16.0)) + 1000.0) + 0.5;
	vec3 clouds = vec3(0.0);

	vec3 col = vec3(0.0);

	if (r < 0.9) {
		if (max(0.0f, noiseVal + 0.75f) < water && !gasGiant) {
			if (frozen) {
				col = vec3(1.0, 1.0, 1.0);
				normal = getBumpNormal(uv, f, 5.0, vec2(0.0), bumpRand);
			}
			else col = vec3(0.0, 0.0, 1.0);
		}
		else {
			col = vec3(color.r, color.g, color.b) * noiseVec;
			if (gasGiant) {
				col += cloudNoise;
				col *= smoothstep(0.9, 0.8, r);
				alpha = smoothstep(0.9, 0.8, r);
			}
			else {
				normal = getBumpNormal(uv, f, 20.0, vec2(0.0), bumpRand);
			}
		}
		if (atmosphere) clouds += smoothstep(0.55, 0.6, cloudNoise) * cloudColor;
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
	float light = dot(normal, vec3(sunVec, 0.0));

	if (length(clouds) > 0.0) {
		light = dot(vec3(uv, 0.0), vec3(sunVec, 0.0));
	}

	col += clouds;
	col *= light;
	col += light / 2.0;

	gl_FragColor = vec4(col, alpha);
}