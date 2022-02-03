in vec2 uv2;
in vec4 color;

void main() {
	gl_FragColor = color * smoothstep(1.0, 0.0, uv2.x) * smoothstep(1.75, 0.75, uv2.y) * smoothstep(-0.5, 0.5, uv2.y);
}