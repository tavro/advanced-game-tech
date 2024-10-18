#version 150

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelToWorldMatrix;

in vec3 in_Position;

void main() {
    gl_Position = projMatrix * viewMatrix * modelToWorldMatrix * vec4(in_Position, 1.0);
}
