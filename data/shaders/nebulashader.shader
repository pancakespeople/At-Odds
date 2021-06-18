#version 130

#define NUM_OCTAVES 5

in vec4 vertPos;
in vec4 color;

uniform sampler2D background;
uniform vec2 size;
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
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	
	float r = fbm(pixel * 1.75 + 0.5);
	float g = fbm((pixel + 50.0) * 1.75 + 0.5);
	float b = fbm((pixel + 100.0) * 1.75 + 0.5);

	vec4 neb1 = vec4(r, 0.0, 0.0, r);
	vec4 neb2 = vec4(0.0, g, 0.0, g);
	vec4 neb3 = vec4(0.0, 0.0, b, b);

	gl_FragColor = color * texture(background, pixel) + neb1 + neb2 + neb3;
}