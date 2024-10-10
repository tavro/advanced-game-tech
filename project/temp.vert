#version 150

in vec3 in_Position;

uniform mat4 viewMatrix, modelToWorldMatrix;
uniform mat4 projMatrix; 

void main(void)
{
    gl_Position = projMatrix * viewMatrix * modelToWorldMatrix * vec4(in_Position, 1.0);
}