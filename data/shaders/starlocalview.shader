#define NUM_OCTAVES 5

uniform float time;
uniform float randSeed;

in vec2 uv2;
in vec4 color;

vec2 random2(vec2 st) {
    st = vec2(dot(st, vec2(127.1, 311.7)),
        dot(st, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(st) * 43758.5453123 * randSeed);
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

float fbm(vec2 x) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(x);
        x = rot * x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

mat2 rotate(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main()
{
    vec2 uv = uv2 * 7.5;
    float dist = length(uv);
    float angle = atan(uv.y, uv.x);
    float c = fbm(vec2(cos(angle) + time / 7.0, sin(angle + dist) - time / 7.0) / 2.0) * 2.0;
    float f = (1.0 - sqrt(1.0 - dist)) / dist;
    float alpha = 0.0;

    vec3 col = vec3(0.0);

    vec2 nuv = uv;
    nuv *= rotate(time / 10.0);

    // Star
    //col += color.xyz * smoothstep(0.17, 0.15, dist);
    col += max((noise(nuv * 25.0 * f) * 0.5 + 0.45) * smoothstep(0.16, 0.15, dist) * color.xyz, 0.0);
    col += max((noise(nuv * 25.0 * f + 50.0 + time / 5.0) * 0.5 + 0.45) * smoothstep(0.16, 0.15, dist) * color.xyz, 0.0);
    alpha += smoothstep(0.16, 0.15, dist);

    // Flaring
    col += vec3(smoothstep(1.0, c, dist)) * color.xyz * smoothstep(0.15, 0.16, dist) / 2.0;

    // Spikes
    //col += vec3(0.1 / abs(uv.y)) * smoothstep(2.0, -5.0, dist) * vec3(0.7, 0.9, 1.0);

    // Glow
    col += vec3(smoothstep(1.0, 0.0, dist / 4.0) / 3.0) * color.xyz * smoothstep(0.15, 0.16, dist);

    // Stars
    //col += vec3(0.01 / length(gv));

    // Output to screen
    gl_FragColor = vec4(col, length(col) + alpha);
}