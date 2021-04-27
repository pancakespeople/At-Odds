#version 130

in vec4 vertPos;
in vec4 color;

uniform float radius = 50;
uniform vec2 resolution;
uniform float time;
uniform bool flashing = false;

void main() {
	vec2 pixel = gl_FragCoord.xy / resolution;
	float dist = distance(pixel, vertPos.xy - radius);	

	if (flashing) {
		gl_FragColor = color - (dist / radius) * vec4(sin(time * 7.5), sin(time * 7.5), sin(time * 7.5), 1.0);
	}
	else {
		gl_FragColor = color - (dist / radius);
	}
}