#define NUM_OCTAVES 5

uniform float seed;
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
    float a = 0.5 * seed;
    vec2 shift = vec2(100);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(x);
        x = rot * x * 2.0 + shift;
        a *= 0.7;
    }
    return v;
}

mat2 rotate(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

vec3 getBumpNormal(vec2 uv, float f, float noiseScale, vec2 offset, float diff) {
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
    float bumpRand = random2(vec2(seed, seed)).x * 0.005 + 0.005;

    float reflectivity = 1.0;
    float alpha = 1.0;
    float emissive = 0.0;

    float noiseVal = fbm((uv * f * 5.0) + 500.0) + 0.5;
    vec3 col = vec3(0.0);

    if (r < 1.0) {
        if (noiseVal < 0.35) {
            col = vec3(1.0, 0.0, 0.0);
            emissive = 0.5;
        }
        else if (noiseVal > 0.65) {
            col = vec3(1.0, 1.0, 1.0) * noiseVal;
            normal = getBumpNormal(uv, f, 20.0, vec2(0.0), bumpRand);
        }
        else {
            col = vec3(noiseVal / 4.0, noiseVal / 4.0, noiseVal / 4.0);
            normal = getBumpNormal(uv, f, 20.0, vec2(0.0), bumpRand);
        }
        reflectivity = noiseVal * 2.0;
    }
    else {
        col = vec3(0.8, 0.3, 0.3) * smoothstep(1.1, 1.0, r);
        alpha = smoothstep(1.1, 1.0, r);
        reflectivity = 0.5;
    }

    float light = dot(normal, vec3(sun, 0.0)) + emissive;
    col *= light;
    col += smoothstep(1.1, 1.0, r) * light * reflectivity * vec3(1.0, 0.5, 0.5);

    if (light < 0.01 && r < 1.0) col = vec3(0.0);

    col += smoothstep(0.75, 0.0, noiseVal) * vec3(1.0, 0.0, 0.0);

    gl_FragColor = vec4(col, alpha);
}