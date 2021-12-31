
uniform sampler2D screen;
uniform float aspect;
uniform vec2 pos;
uniform float zoom;

void main() {
	vec2 uv = gl_TexCoord[0].xy;
	vec2 bhUv = vec2(uv.x * aspect, uv.y);

	pos = vec2(pos.x * aspect, pos.y);

	float radius = length(bhUv - pos) * zoom / 10.0;
	float angle = atan(bhUv.y - pos.y, bhUv.x - pos.x);
	float distortion = 0.1 / radius;

	uv -= distortion * vec2(cos(angle), sin(angle));

	vec4 col = texture(screen, uv);
	col *= smoothstep(1.0, 0.9, distortion);

	gl_FragColor = col;
}