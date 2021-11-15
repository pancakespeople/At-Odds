in vec2 uv2;

uniform float seed;
uniform float time;

vec2 random2(vec2 st) {
    st = vec2(dot(st, vec2(127.1, 311.7)),
        dot(st, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(st) * 43758.5453123 * seed);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(dot(random2(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
        dot(random2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
        mix(dot(random2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
            dot(random2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x), u.y);
}

mat2 rotate(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main()
{
    vec2 uv = uv2;
    float radius = length(uv);
    float ringDensity = 800.0;
    float ring = sin(radius * ringDensity) + 1.0;
    float ringIndex = round(ring);
    float rotationSpeed = 0.05 * ringIndex * ceil(radius / 3.14159 * ringDensity);
    float n = noise(uv * 250.0 * rotate(time / rotationSpeed));
    float v = ring * smoothstep(0.4, 0.3, radius) * smoothstep(0.1, 0.2, radius) * n;

    vec3 col = vec3(v);

    gl_FragColor = vec4(col, v);
}