
void main() {
	float radius = length(gl_TexCoord[0].xy-0.5);
	gl_FragColor = 0.1 / radius * gl_Color * smoothstep(0.5, 0.0, radius);
}