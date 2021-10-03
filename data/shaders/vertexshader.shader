#version 130

out vec4 vertPos;
out vec4 color;
out vec2 uv;

uniform vec2 size;

void main() {
    // transform the vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    vertPos = gl_Vertex;
    color = gl_Color;
    uv = vertPos.xy - size;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}