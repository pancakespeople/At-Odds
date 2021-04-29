#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 resolution;

void main() {
	vec2 pixel = gl_FragCoord.xy / resolution;
	float dist = distance(pixel, vertPos.xy);

	// float coolTime = tan( (dist - time * 2500) / 1000);

	gl_FragColor = color - vec4(0, 0, 0, dist / 15000); //+ vec4(coolTime, 0, coolTime, 0);
}