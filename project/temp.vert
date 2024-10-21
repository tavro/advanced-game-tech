#version 150

in vec3 in_Position;
in vec3 in_TexCoord;

uniform mat4 viewMatrix, modelToWorldMatrix;
uniform mat4 projMatrix; 

out vec2 outTexCoord;

void main(void)
{
    outTexCoord.x = in_TexCoord.x;
    outTexCoord.y = in_TexCoord.y;
    
    gl_Position = projMatrix * viewMatrix * modelToWorldMatrix * vec4(in_Position, 1.0);
}