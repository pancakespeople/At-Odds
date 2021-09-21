#version 130

uniform vec2 size;
uniform int numPoints;
uniform vec2 points[50];
uniform vec3 color;
in vec4 vertPos;

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	vec2 worldPos = vertPos.xy - size;
	
	float val = 1.0;

	for (int i = 0; i < numPoints; i++) {
		float dist = distance(worldPos, points[i] - size);
		val *= numPoints * 100.0 / dist;
	}

	if (val > 1.0 && val < 1.1) gl_FragColor = vec4(color, 1.0);
	if (val > 1.1) gl_FragColor = vec4(color - 0.5, 0.5);
}