#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
uniform sampler2D bloomTex;
out vec4 out_Color;

void main(void)
{
    vec4 original = texture(texUnit, outTexCoord);
    vec4 bloom = texture(bloomTex, outTexCoord);
    out_Color = original + bloom;
}
