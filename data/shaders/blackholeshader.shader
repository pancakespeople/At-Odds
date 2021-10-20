#define NUM_OCTAVES 5

in vec2 uv2;
uniform float time;
uniform float randSeed;
uniform vec2 size;

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

void main()
{
	float bhRadius = 0.1;
	float n = fbm(uv2 * 2.0 * rotate(time / 4.0)) + 0.8;
	float dist = length(uv2);

	vec3 col = vec3(smoothstep(0.35, 0.0, dist - bhRadius));
	col += smoothstep(0.3, 0.0, dist - bhRadius) * vec3(1.0, 0.8, 0.5);
	col *= smoothstep(bhRadius, bhRadius + 0.01, dist) * n;
	col *= fbm((uv2 + 1.0 / (dist + bhRadius)) * 2.0 * (dist - bhRadius) * rotate(time / 16.0)) + 0.8;

	float alpha = length(col);
	alpha += smoothstep(bhRadius + 0.01, bhRadius, dist);

	gl_FragColor = vec4(col, alpha);
}