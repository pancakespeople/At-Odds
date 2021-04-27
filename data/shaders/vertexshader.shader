#version 130

out vec4 vertPos;
out vec4 color;

uniform vec2 resolution;

void main() {
    // transform the vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    vertPos = gl_Vertex;
    color = gl_Color;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}