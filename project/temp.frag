#version 150

out vec4 out_Color;
uniform vec3 floorColor;

uniform int isTex;
uniform sampler2D texUnit;
in vec2 outTexCoord;
in vec3 out_Normal;

void main(void)
{
    if(isTex == 1) {
        out_Color = texture(texUnit, outTexCoord);
    }
    else {
        out_Color = vec4(floorColor, 1.0);
    }
}
