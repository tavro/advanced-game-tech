#version 150

//in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;
uniform mat4 matrix;

// NOTE: I added this
uniform vec3 bonePos[2];
uniform mat4 boneRot[2];

out vec4 g_color;
const vec3 lightDir = normalize(vec3(0.3, 0.5, 1.0));

// Uppgift 3: Soft-skinning p� GPU
//
// Flytta �ver din implementation av soft skinning fr�n CPU-sidan
// till vertexshadern. Mer info finns p� hemsidan.

void main(void)
{
    // NOTE: I added this
	float weight0 = in_TexCoord.x;
    float weight1 = in_TexCoord.y;

    vec3 posBone0 = (boneRot[0] * vec4(in_Position, 1.0)).xyz + bonePos[0];
    vec3 posBone1 = (boneRot[1] * vec4(in_Position, 1.0)).xyz + bonePos[1];
    vec3 finalPos = posBone0 * weight0 + posBone1 * weight1;
    
    // transformera resultatet med ModelView- och Projection-matriserna
	gl_Position = matrix * vec4(finalPos, 1.0);

	// s�tt r�d+gr�n f�rgkanal till vertex Weights
	vec4 color = vec4(in_TexCoord.x, in_TexCoord.y, 0.0, 1.0);

	// L�gg p� en enkel ljuss�ttning p� vertexarna 	
	float intensity = dot(in_Normal, lightDir);
	color.xyz *= intensity;

	g_color = color;
}

