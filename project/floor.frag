#version 150

uniform vec3 floorColor;

out vec4 out_Color;

void main() {
    out_Color = vec4(floorColor, 1.0);
}
