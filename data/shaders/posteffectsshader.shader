
uniform sampler2D screen;
uniform float aspect;
uniform vec2 blackHolePos;
uniform float zoom;
uniform bool blackHole;
uniform float time;

uniform vec2 explosionPoints[64];
uniform float explosionTimes[64];
uniform int numExplosions;


void main() {
	vec2 uv = gl_TexCoord[0].xy;
	vec2 newUv = vec2(uv.x * aspect, uv.y);
	vec4 col = texture(screen, uv);
	vec4 light = vec4(0.0);

	// Explosions
	for (int i = 0; i < numExplosions; i++) {
		vec2 explosionPos = vec2(explosionPoints[i].x * aspect, explosionPoints[i].y);
		float explTime = time - explosionTimes[i];
		float radius = length(newUv - explosionPos) * zoom + 0.1 - explTime;
		float angle = atan(newUv.y - explosionPos.y, newUv.x - explosionPos.x);
		float distortion = smoothstep(0.1, 0.2, radius) * smoothstep(0.3, 0.2, radius) * smoothstep(0.0, 1.0, radius) - explTime / 20.0;
		light += clamp(vec4(1.0, 0.8, 0.7, 1.0) * smoothstep(0.5, 0.0, radius) * 1.75 - explTime, 0.0, 1.0);

		uv += clamp(distortion, 0.0, 100.0) * vec2(cos(angle), sin(angle));
	}

	// Black hole
	float blackHoleDistortion = 0.0;

	if (blackHole) {
		blackHolePos = vec2(blackHolePos.x * aspect, blackHolePos.y);

		float radius = length(newUv - blackHolePos) * zoom / 10.0;
		float angle = atan(newUv.y - blackHolePos.y, newUv.x - blackHolePos.x);
		blackHoleDistortion = 0.1 / radius;
		blackHoleDistortion *= smoothstep(0.6, 0.5, radius);

		uv -= blackHoleDistortion * vec2(cos(angle), sin(angle));
	}

	col = texture(screen, uv);
	col *= smoothstep(0.6, 0.5, blackHoleDistortion);
	col += clamp(light, 0.0, 1.0);

	gl_FragColor = col;
}