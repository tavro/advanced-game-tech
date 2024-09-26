#version 150
// bump mapping should be calculated
// 1) in view coordinates
// 2) in texture coordinates

in vec2 outTexCoord;
in vec3 out_Normal;
in vec3 Ps;
in vec3 Pt;
in vec3 pixPos;  // Needed for specular reflections
uniform sampler2D texUnit;
uniform sampler2D bumpMap;
out vec4 out_Color;

void main(void)
{
    vec3 light = normalize(vec3(0.0, 0.7, 0.7)); // Light source in view coordinates
	
	// Calculate gradients here
	float offset = 1.0 / 256.0; // texture size, same in both directions
	
	float Bs = texture(bumpMap, outTexCoord + vec2(offset, 0.0)).r - texture(bumpMap, outTexCoord - vec2(offset, 0.0)).r;
    float Bt = texture(bumpMap, outTexCoord + vec2(0.0, offset)).r - texture(bumpMap, outTexCoord - vec2(0.0, offset)).r;

	// Modify the normal using the tangent and bitangent (Ps, Pt)
    float bumpScale = 2.0;
	vec3 modifiedNormal = normalize(out_Normal + bumpScale * (Ps * Bs + Pt * Bt));
    float diffuse = max(dot(modifiedNormal, light), 0.0);

	// Simplified lighting calculation.
	// A full solution would include material, ambient, specular, light sources, multiply by texture.
    out_Color = vec4(vec3(diffuse), 1.0);
}
