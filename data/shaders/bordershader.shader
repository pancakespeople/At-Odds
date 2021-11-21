#version 130

uniform vec2 size;
uniform int numPoints;
uniform vec3 points[256];
uniform vec3 color;
in vec4 vertPos;

void main() {
	vec2 worldPos = vertPos.xy - size;
	
    vec2 closestPoint = points[0].xy;
    int closestIndex = 0;
    float closestDist = distance(points[0].xy - size, worldPos);
    vec4 col = vec4(color, 1.0) * (1.0 - closestDist / 750.0);

    for (int i = 0; i < numPoints; i++) {
        float dist = distance(points[i].xy - size, worldPos);
        if (dist < closestDist) {
            closestPoint = points[i].xy - size;
            closestDist = dist;
            closestIndex = i;
            col = vec4(color, 1.0) * (1.0 - closestDist / 750.0);
        }

    }

    if (points[closestIndex].z == 0.0) col = vec4(0.0);
    
    gl_FragColor = col;
}