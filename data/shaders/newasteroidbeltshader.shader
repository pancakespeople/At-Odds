
uniform sampler2D tex;
uniform vec2 sunPos;
uniform float time;

in vec2 pos;
in float rotationAngle;

mat2 rotate(float angle) {
	float c = cos(angle);
	float s = sin(angle);
	return mat2(c, -s, s, c);
}

void main() {
	
	sunPos.y = -sunPos.y;
	pos.y = -pos.y;
	
	vec2 uv = gl_TexCoord[0].xy;
	
	vec4 texCol = texture(tex, uv);

	vec3 toSun = normalize(vec3(vec2(sunPos - pos) * rotate(rotationAngle), 0.0));
	vec3 normal = (texCol.rgb - 0.5) * 2.0;
	float alpha = texCol.a;

	float light = dot(normal, toSun);

	gl_FragColor = vec4(vec3(light), alpha);
}