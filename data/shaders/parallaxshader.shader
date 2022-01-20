
in vec2 uv2;
uniform vec2 size;
uniform vec2 cameraPos;
uniform float zoom;

vec2 random2(vec2 st) {
    st = vec2(dot(st, vec2(127.1, 311.7)),
        dot(st, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(st) * 43758.5453123);
}

float random(float x) {
    return fract(sin(28572.0 * x + 56847.0));
}

vec4 layer(vec2 uv) {
    vec2 gridPos = floor(uv * 2.0);
    vec2 gv = fract(uv * 2.0);

    vec4 col = vec4(0.0);
    for (float x = -2.0; x <= 2.0; x++) {
        for (float y = -2.0; y <= 2.0; y++) {
            vec2 offset = vec2(x, y);

            vec2 pos = random2(gridPos + offset);
            float brightness = random(pos.x + pos.y) * 0.01;
            vec2 newUv = gv - pos - offset;
            
            col += brightness / length(newUv) * smoothstep(1.0, 0.5, length(newUv));
        }
    }
    return col;
}

void main()
{
    vec2 uv = uv2;
    uv.x *= size.x / size.y;
    uv *= zoom;

    vec4 col = vec4(0.0);

    for (float i = 0.0; i < 1.0; i += 1.0/4.0) {
        float depth = i;
        vec2 newUv = uv * depth + random(i);
        
        newUv.x += cameraPos.x / 10000.0;
        newUv.y += cameraPos.y / 10000.0;

        col += layer(newUv) * smoothstep(0.0, 0.1, depth);
    }

    gl_FragColor = col;
}