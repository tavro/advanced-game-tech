// SpriteLight - Heavily simplified sprite engine
// by Ingemar Ragnemalm 2009

// What does a mogwai say when it sees a can of soda?
// Eeek! Sprite light! Sprite light!

#include "LoadTGA.h"
#include "VectorUtils4.h"

typedef struct SpriteRec
{
	vec3 position; // OBS! Really 2D, declared as vec3 to make functions like normalizations compatible.
	TextureData *face;
	vec3 speed;
	GLfloat rotation;
	struct SpriteRec *next;
	
	// Add custom sprite data here as needed
} SpriteRec, *SpritePtr;

// Globals: The sprite list, background texture and viewport dimensions (virtual or real pixels)
extern SpritePtr gSpriteRoot;
extern GLuint backgroundTexID;
extern long gWidth, gHeight;

// Functions
TextureData *GetFace(const char *fileName);
struct SpriteRec *NewSprite(TextureData *f, GLfloat h, GLfloat v, GLfloat hs, GLfloat vs);
void HandleSprite(SpritePtr sp);
void DrawSprite(SpritePtr sp);
void DrawBackground();

void InitSpriteLight();
