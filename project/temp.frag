#version 150

out vec4 out_Color;
uniform vec3 floorColor;

void main(void)
{
    out_Color = vec4(floorColor, 1.0);
}
