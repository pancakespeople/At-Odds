
in vec2 uv2;
uniform float time;
uniform float seed;

mat2 rotate(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main()
{
    vec2 uv = uv2;

    float radius = length(uv);
    uv *= rotate(time * 10.0);
    vec4 col = vec4(0.001 / abs(uv.x * uv.y));
    col *= smoothstep(0.25, 0.0, radius + time);
    col += 0.1 / (radius + time);
    col *= smoothstep(0.5, 0.0, radius);

    gl_FragColor = col * vec4(0.5, 0.0, 1.0, 1.0);
}