#version 130

in vec4 vertPos;
in vec4 color;

uniform vec2 size;
uniform float time;

void main() {
	vec2 pixel = gl_FragCoord.xy / size;
	
	vec2 toCenter = vertPos.xy - size / 2.0;
	float radius = distance(toCenter, pixel) * 2.0;
	//float angle = atan(toCenter.y, toCenter.x);

	vec4 newColor = color;

	//vec4 adder = vec4(0.0, sin(time*angle/sqrt(radius)) / (radius / (size.x / 2.0)), 0.0, 0.0);
	//newColor += adder;

	float change = abs(sin(time/10.0)) * 250.0;
	
	vec3 adder2 = vec3(pow(sin(time/10.0), 2.0) * 0.5);

	gl_FragColor = newColor - vec4(0.0, 0.0, 0.0, (radius - 1000.0 + change) / (size.x / 2.0)) + vec4(adder2, 0.0);
}