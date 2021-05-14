#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 size;
uniform float time;
uniform float randSeed;

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
	
	vec2 worldPos = vertPos.xy - size / 2.0;
	float radius = distance(worldPos, pixel) * 2.0;
	//float angle = atan(toCenter.y, toCenter.x);

	vec4 newColor = color;

	//vec4 adder = vec4(0.0, sin(time*angle/sqrt(radius)) / (radius / (size.x / 2.0)), 0.0, 0.0);
	//newColor += adder;

	float change = abs(sin(time/10.0)) * 250.0;
	vec3 adder2 = vec3(pow(sin(time/10.0), 2.0) * 0.5);

	vec3 noiseColor;

	if (radius - 1000.0 + change <= 1.0) {
		vec2 noisePos = (worldPos*sqrt(radius)) / 2500.0;
		//vec2 noisePos = worldPos / 100.0;
		vec2 angleVector = random2(vec2(randSeed));
		noisePos.x += angleVector.x * time;
		noisePos.y += angleVector.y * time;
		noiseColor = vec3(noise(noisePos * 0.5 + 0.5));
	}
	else {
		noiseColor = vec3(0.0, 0.0, 0.0);
	}

	gl_FragColor = newColor - vec4(0.0, 0.0, 0.0, (radius - 1000.0 + change) / (size.x / 2.0)) + vec4(adder2, 0.0) + vec4(noiseColor, 0.0);
}