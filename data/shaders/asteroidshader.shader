uniform vec2 toSun;
uniform sampler2D tex;

in vec4 color;

void main() {
	vec2 uv = gl_TexCoord[0].xy;

	vec4 texCol = texture(tex, uv);
	float alpha = texCol.a;
	vec3 normal = (texCol.rgb - 0.5) * 2.0;

	float light = dot(normal, vec3(toSun, 0.0));

	gl_FragColor = vec4(color.rgb * light, alpha);
}