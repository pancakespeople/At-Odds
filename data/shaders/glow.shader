#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 size;

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	float dist = distance(pixel, vertPos.xy - size / 2.0);

	gl_FragColor = color - vec4(0, 0, 0, dist / (size.x / 2.0));
}