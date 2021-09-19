#version 130

uniform vec2 size;
uniform float time;
in vec4 vertPos;

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	vec2 worldPos = vertPos.xy - size;
	float angle = atan(worldPos.y, worldPos.x);

	float m = mod(angle - time, 3.14159 / 1.5);

	if (m < 1.0)
		gl_FragColor = gl_Color;
}