#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    vec2 texOffset = 1.0 / textureSize(texUnit, 0);
    vec4 color = vec4(0.0);
    int kernelSize = 5;
    float kernelWeight = 1.0 / float(kernelSize * kernelSize);
    
    for (int x = -kernelSize / 2; x <= kernelSize / 2; ++x) {
        for (int y = -kernelSize / 2; y <= kernelSize / 2; ++y) {
            color += texture(texUnit, outTexCoord + vec2(x, y) * texOffset) * kernelWeight;
        }
    }
    
    out_Color = color;
}
