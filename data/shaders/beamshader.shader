uniform float time;

in vec4 color;

float rand(float n) { return fract(sin(n) * 43758.5453123); }

float noise(float p) {
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

void main() {
    vec2 uv = gl_TexCoord[0].xy - 0.5;

    float val = smoothstep(0.5 + sin(uv.x * 50.0 + noise(uv.x * 100.0 - time * 10.0) - time * 20.0) / 5.0, 0.0, abs(sin(uv.y))) * smoothstep(0.5, 0.0, abs(uv.x));
    vec3 col = val * color;

    gl_FragColor = vec4(col, val);
}