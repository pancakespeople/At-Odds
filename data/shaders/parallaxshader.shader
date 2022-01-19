
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
            float brightness = random(pos.x + pos.y) * 0.005;
            vec2 newUv = gv - pos - offset;
            //float angle = atan(newUv.y, newUv.x);
            //float light = dot(vec2(cos(iTime), sin(iTime)), vec2(cos(angle), sin(angle)));

            col += brightness / length(newUv) * smoothstep(1.0, 0.5, length(newUv));
        }
    }
    return col;
}

void main()
{
    vec2 uv = uv2;
    uv.x *= size.x / size.y;

    uv *= 2.0;

    vec4 col = vec4(0.0);

    for (float i = 0.0; i < 4.0; i++) {
        vec2 newUv = (uv + random(i)) * 2.0;
        newUv.x += cameraPos.x / 10000.0 * random(i);
        newUv.y += cameraPos.y / 10000.0 * random(i);

        col += layer(newUv); //* smoothstep(15.0, 10.0, zoom);
    }


    gl_FragColor = col;
}