#version 150

in  vec3  in_Position;
in  vec3  in_Normal;
in  vec2  in_TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 modelToWorldToView;

out float shade;

void main(void)
{
	shade = (mat3(modelToWorldToView)*in_Normal).z; // Fake shading
	gl_Position=projectionMatrix*modelToWorldToView*vec4(in_Position, 1.0);
}

