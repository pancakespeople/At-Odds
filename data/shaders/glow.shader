#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 size;

float gauss(float x) {
	return exp(-((pow(x, 2.0) / 2.0)));
}

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	float dist = distance(pixel, vertPos.xy - size / 2.0);
	float mod = gauss(dist * 5.0 / size.y);

	if (mod > 0.01) {
		gl_FragColor = color * gauss(dist * 5.0 / size.y);
	}
	else {
		gl_FragColor = vec4(0.0);
	}
}