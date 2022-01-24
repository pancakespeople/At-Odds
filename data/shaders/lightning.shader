
in vec2 uv2;
uniform float time;

vec2 random2(vec2 st) {
	st = vec2(dot(st, vec2(127.1, 311.7)),
		dot(st, vec2(269.5, 183.3)));
	return -1.0 + 2.0 * fract(sin(st) * 43758.5453123 * 0.7897);
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

float random(float x) {
	return fract(sin(28572.0 * x + 56847.0));
}

float strike(float cycleTime, float strikeTime) {
	return abs(1.0 / (cycleTime - strikeTime)) - 1.0;
}

void main()
{
	vec2 uv = uv2;
	vec2 np = vec2(uv.x, uv.y) * 10.0;
	float numStrikes = 3.0;
	float realRadius = length(uv);

	vec4 col = vec4(0.0);

	for (float i = 0.0; i < numStrikes; i++) {
		float radius = length(uv) * fbm(np + (100.0 * i)) * 20.0;
		float strikeTime = random(i) * 8.0 + 1.0;
		col += max(abs(0.1 / (radius - 0.25)) * vec4(0.7, 0.8, 1.0, 0.5) * strike(time, strikeTime), 0.0);
	}

	col *= smoothstep(0.25, 0.0, realRadius);

	gl_FragColor = col;
}