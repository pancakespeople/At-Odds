in vec2 uv2;
uniform float time;
uniform float seed;

vec2 random2(vec2 st) {
	st = vec2(dot(st, vec2(127.1, 311.7)),
		dot(st, vec2(269.5, 183.3)));
	return -1.0 + 2.0 * fract(sin(st) * 43758.5453123 * seed);
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

void main()
{
	vec2 uv = uv2;
	float n2 = fbm((uv + 100.0) * 2.0 + vec2(time / 10.0, -time / 10.0));
	float n3 = fbm((uv + 200.0) * 2.0 + vec2(-time / 10.0, time / 10.0));
	float radius = length(uv);
	float n = fbm((uv + n2 + n3) * (2.0 / radius)) + 0.75;

	vec4 col = vec4(n * 2.0) * vec4(0.5, 0.0, 1.0, 1.0);
	col *= smoothstep(0.5, 0.0, radius);

	col += abs(0.01 / (radius - 0.2 - n/3.0)) * vec4(0.5, 0.0, 1.0, 1.0);
	col *= smoothstep(0.5, 0.4, radius);

	gl_FragColor = col;
}