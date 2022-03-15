#version 130

uniform float time;

void main() {
	vec2 uv = gl_TexCoord[0].xy - 0.5;
	float angle = atan(uv.y, uv.x) - time;
	float dist = length(uv);

	float m = 0.25 / abs(sin(angle) * cos(angle));

	gl_FragColor = gl_Color * smoothstep(0.25, 1.0, m) * smoothstep(0.5, 0.45, dist) - smoothstep(0.4, 0.35, dist);
}