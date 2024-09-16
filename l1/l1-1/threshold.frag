#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    vec4 color = texture(texUnit, outTexCoord);
    float threshold = 1.0;
    if (max(color.r, max(color.g, color.b)) > threshold)
        out_Color = color;
    else
        out_Color = vec4(0.0);
}