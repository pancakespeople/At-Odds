#version 130

uniform vec2 size;
uniform float time;
in vec4 vertPos;

void main() {
	vec2 uv = (vertPos.xy - size / 2.0) / size;
	float angle = atan(uv.y, uv.x) - time;
	float dist = length(uv);

	float m = 0.25 / abs(sin(angle) * cos(angle));

	gl_FragColor = gl_Color * smoothstep(0.25, 1.0, m) * smoothstep(0.5, 0.45, dist) - smoothstep(0.4, 0.35, dist);
}