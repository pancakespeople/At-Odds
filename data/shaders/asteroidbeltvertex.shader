uniform float time;
uniform float rotationSpeed;
uniform vec2 sunPos;

varying out vec2 pos;
varying out float rotationAngle;
varying out vec4 color;

mat2 rotate(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main() {
    // transform the vertex position
    rotationAngle = time * rotationSpeed;
    
    gl_Vertex.xy *= rotate(rotationAngle);
    gl_Vertex.xy += sunPos;
    
    pos = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
    color = gl_Color;
}