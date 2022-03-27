#version 130

#define NUM_OCTAVES 5

uniform float time;
uniform float seed;
uniform vec2 size;
uniform vec2 sun;

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

vec3 getBumpNormal(vec2 uv, float f, float noiseScale, vec2 offset) {
    float diff = 0.005;
    vec2 uv2 = uv - offset;

    float heightX = fbm(vec2(uv2.x + diff, uv2.y) * f * noiseScale);
    float heightY = fbm(vec2(uv2.x, uv2.y + diff) * f * noiseScale);
    float heightX2 = fbm(vec2(uv2.x - diff, uv2.y) * f * noiseScale);
    float heightY2 = fbm(vec2(uv2.x, uv2.y - diff) * f * noiseScale);

    vec2 heightDiff = vec2(heightX - heightX2, heightY - heightY2);
    float z = sqrt(1.0 - pow(length(heightDiff), 2.0));

    return vec3(heightDiff + uv, z);
}

void main()
{
    vec2 uv = (gl_TexCoord[0].xy - 0.5) * 2.25;

    float r = length(uv);
    float f = (1.0 - sqrt(1.0 - r)) / r;
    vec3 normal = vec3(uv, 0.0);
    vec3 toLight = vec3(sun, 0.0);
    float reflectivity = 1.0;
    float alpha = 1.0;

    float noiseVal = fbm((uv * f * 5.0) + 500.0) + 0.5;
    float cloudNoise = fbm((uv * f * 2.0 * rotate(time / 16.0)) + 1000.0) + 0.5;
    float clouds = smoothstep(0.55, 0.6, cloudNoise);
    vec3 col = vec3(0.0);

    if (r < 1.0) {
        if (noiseVal < 0.5) {
            col = vec3(0.0, 0.0, 1.0);
            normal = getBumpNormal(uv, f, 10.0, vec2(fbm(uv + (time / 10.0))) * 0.5);
        }
        else if (noiseVal > 0.65) {
            col = vec3(1.0, 1.0, 1.0) * noiseVal;
            normal = getBumpNormal(uv, f, 20.0, vec2(0.0));
        }
        else {
            col = vec3(0.5, noiseVal, 0.0);
            normal = getBumpNormal(uv, f, 20.0, vec2(0.0));
        }
        reflectivity = noiseVal * 2.0;
    }
    else {
        col = vec3(0.5, 0.8, 1.0) * smoothstep(1.1, 1.0, r);
        reflectivity = 0.5;
        alpha = smoothstep(1.1, 1.0, r);
    }


    float light = dot(normal, toLight);

    if (clouds > 0.0) {
        light = dot(vec3(uv, 0.0), toLight);
    }

    col += clouds;
    col *= light;

    col += smoothstep(1.1, 1.0, r) * light * reflectivity;

    gl_FragColor = vec4(col, alpha);
}