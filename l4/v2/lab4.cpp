// Variant with SimpleGUI

#include <stdlib.h>
#include "MicroGlut.h"
#include "LoadTGA.h"
#include "SpriteLight.h"
#include "GL_utilities.h"
#define MAIN
#include "VectorUtils4.h"
#include "SimpleGUI.h"

// Add more globals as needed

// Example of user controllable parameter
float someValue = 1.0;

void SpriteBehavior() // Your code!
{
// Add your lab code here. You may edit anywhere you want, but most of it goes here.
// You can start from the global list gSpriteRoot.
}

// Drawing routine
void Display()
{
	SpritePtr sp;
	
	glClearColor(0, 0, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	DrawBackground();

	SpriteBehavior(); // Your code
	
// Loop though all sprites. (Several loops in real engine.)
	sp = gSpriteRoot;
	do
	{
		// Your code
		// Example affecting sprites by a controllable parameter
		sp->speed = normalize(sp->speed) * someValue;
		
		HandleSprite(sp); // Default movement my speed. Callback in a real engine
		DrawSprite(sp);
		sp = sp->next;
	} while (sp != NULL);

      	sgDraw();

	glutSwapBuffers();
}

void Reshape(int h, int v)
{
	glViewport(0, 0, h, v);
	gWidth = h;
	gHeight = v;
}

void Key(unsigned char key,
         __attribute__((unused)) int x,
         __attribute__((unused)) int y)
{
  switch (key)
  {
    case '+':
    	someValue += 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case '-':
    	if (someValue > 0.5) someValue -= 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case 0x1b:
      exit(0);
  }
}

void Init()
{
	TextureData *sheepFace, *blackieFace, *dogFace, *foodFace;
	
	LoadTGATextureSimple("bilder/leaves.tga", &backgroundTexID); // Background
	
	sheepFace = GetFace("bilder/sheep.tga"); // A sheep
	blackieFace = GetFace("bilder/blackie.tga"); // A black sheep
	dogFace = GetFace("bilder/dog.tga"); // A dog
	foodFace = GetFace("bilder/mat.tga"); // Food
	
	NewSprite(sheepFace, 100, 200, 1, 1);
	NewSprite(sheepFace, 200, 100, 1.5, -1);
	NewSprite(sheepFace, 250, 200, -1, 1.5);
	
	sgCreateStaticString(20, 40, "Slider and float display");
	sgCreateSlider(-1, -1, 200, &someValue, 0.5, 5);
	sgCreateDisplayFloat(-1, -1, "Value: ", &someValue);
	
	// Always fix the colors if it looks bad.
        sgSetFrameColor(0,0,0);
        sgSetBackgroundColor(1, 1, 1, 0.5);
        sgSetTextColor(0, 0, 0);	
}

void mouse(int button, int state, int x, int y)
{
	sgMouse(state, x, y);
	glutPostRedisplay();
}

void drag(int x, int y)
{
	sgMouseDrag(x, y);
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 2);
	glutCreateWindow("Lab 4 Flocking");
	
	glutDisplayFunc(Display);
	glutRepeatingTimer(20); // Should match the screen synch
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutMouseFunc(mouse);
	glutMotionFunc(drag);
	
	InitSpriteLight();
	Init();
	
	glutMainLoop();
	return 0;
}
