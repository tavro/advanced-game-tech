#include <stdlib.h>
#include "MicroGlut.h"
#include "LoadTGA.h"
#include "SpriteLight.h"
#include "GL_utilities.h"
#define MAIN
#include "VectorUtils4.h"

// Add more globals as needed
// NOTE: I added this
TextureData *foodFace;
typedef struct {
    vec3 position;
    int active;
    float timer;
} Food;

Food food;
float foodAttractionRadius = 300.0f;
float foodDuration = 1.0f;
float foodAttractionStrength = 0.2f;

// NOTE: I added this
float cohesionRadius = 200.0f;
float cohesionStrength = 0.1f;

float separationRadius = 50.0f;
float separationStrength = 0.5f;

float alignmentRadius = 100.0f;
float alignmentStrength = 0.25f;

float maxSpeed = 3.0f;

// NOTE: I added this
void LimitSpeed(SpritePtr boid, float maxSpeed) {
    float currentSpeed = Norm(boid->speed);
    if (currentSpeed > maxSpeed) {
        boid->speed = ScalarMult(Normalize(boid->speed), maxSpeed);
    }
}

// NOTE: I added this
void Cohesion(SpritePtr boid) {
    SpritePtr otherBoid = gSpriteRoot;
    vec3 sumPosition = SetVector(0.0, 0.0, 0.0);

    int count = 0;
    while (otherBoid != NULL) {
        if (otherBoid != boid) {
            float dist = Norm(VectorSub(boid->position, otherBoid->position));
            if (dist < cohesionRadius) {
                sumPosition = VectorAdd(sumPosition, otherBoid->position);
                count++;
            }
        }
        otherBoid = otherBoid->next;
    }

    if (count > 0) {
        vec3 averagePosition = ScalarMult(sumPosition, 1.0 / count);
        vec3 direction = Normalize(VectorSub(averagePosition, boid->position));
        boid->speed = VectorAdd(boid->speed, ScalarMult(direction, cohesionStrength));	
    }
	LimitSpeed(boid, maxSpeed);
}

// NOTE: I added this
void Separation(SpritePtr boid) {
    SpritePtr otherBoid = gSpriteRoot;
    vec3 sumDirection = SetVector(0.0, 0.0, 0.0);
    
    int count = 0;
    while (otherBoid != NULL) {
        if (otherBoid != boid) {
            float dist = Norm(VectorSub(boid->position, otherBoid->position));
            if (dist < separationRadius && dist > 0.0) {
                vec3 awayDirection = Normalize(VectorSub(boid->position, otherBoid->position));
                sumDirection = VectorAdd(sumDirection, awayDirection);
                count++;
            }
        }
        otherBoid = otherBoid->next;
    }

    if (count > 0) {
        vec3 separationForce = ScalarMult(Normalize(sumDirection), separationStrength);
        boid->speed = VectorAdd(boid->speed, separationForce);
    }
    LimitSpeed(boid, maxSpeed);
}

// NOTE: I added this
void Alignment(SpritePtr boid) {
    SpritePtr otherBoid = gSpriteRoot;
    vec3 sumSpeed = SetVector(0.0, 0.0, 0.0);
    
    int count = 0;
    while (otherBoid != NULL) {
        if (otherBoid != boid) {
            float dist = Norm(VectorSub(boid->position, otherBoid->position));
            if (dist < alignmentRadius) {
                sumSpeed = VectorAdd(sumSpeed, otherBoid->speed);
                count++;
            }
        }
        otherBoid = otherBoid->next;
    }

    if (count > 0) {
        vec3 averageSpeed = ScalarMult(sumSpeed, 1.0 / count);
        vec3 alignmentForce = Normalize(VectorSub(averageSpeed, boid->speed));
        boid->speed = VectorAdd(boid->speed, ScalarMult(alignmentForce, alignmentStrength));
    }
	LimitSpeed(boid, maxSpeed);
}

// NOTE: I added this
void AddNoise(SpritePtr boid) {
    float noiseStrength = 0.9f;
    boid->speed.x += (((rand() % 100) / 50.0f) - 1) * noiseStrength;
    boid->speed.y += (((rand() % 100) / 50.0f) - 1) * noiseStrength;
    boid->speed.z += (((rand() % 100) / 50.0f) - 1) * noiseStrength;
}

// NOTE: I added this
void SpawnFood(float x, float y) {
    food.position = SetVector(x, y, 0.0);
    food.active = 1;
    food.timer = foodDuration;
}

// NOTE: I added this
void FoodAttraction(SpritePtr boid) {
    if (food.active) {
        float dist = Norm(VectorSub(food.position, boid->position));
        if (dist < foodAttractionRadius) {
            vec3 direction = Normalize(VectorSub(food.position, boid->position));
            boid->speed = VectorAdd(boid->speed, ScalarMult(direction, foodAttractionStrength));
            LimitSpeed(boid, maxSpeed);
        }
        
        if (dist < 20.0f) {
            //food.active = 0; // TODO
        }
    }
}

// NOTE: I added this
void UpdateFood(float deltaTime) {
    if (food.active) {
        food.timer -= deltaTime;
        if (food.timer <= 0.0f) {
            food.active = 0;
        }
    }
}

void SpriteBehavior() // Your code!
{
	// Add your lab code here. You may edit anywhere you want, but most of it goes here.
	// You can start from the global list gSpriteRoot.
	// NOTE: I added this
	SpritePtr boid = gSpriteRoot;
    while (boid != NULL) {
        /*
        if(boid->isFood) {
            boid = boid->next;
            continue;
        }
        */

        if (boid->isBlackSheep) {
            AddNoise(boid);
        }

        Cohesion(boid);
        Separation(boid);
        Alignment(boid);

        /*
        FoodAttraction(boid);
        */

        boid->position = VectorAdd(boid->position, boid->speed);
        boid = boid->next;
    }
}

SpritePtr foodSprite = NULL;
// Drawing routine
float lastTime = 0.0f;
void Display()
{
    // NOTE: I added this
    float currentTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

	SpritePtr sp;
	
	glClearColor(0, 0, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	DrawBackground();
	
	SpriteBehavior(); // Din kod!

    UpdateFood(deltaTime);

// Loop though all sprites. (Several loops in real engine.)
	sp = gSpriteRoot;
	do
	{
		HandleSprite(sp); // Callback in a real engine
		if(!sp->isFood) {
            DrawSprite(sp);
        }
		sp = sp->next;
	} while (sp != NULL);
	
    // NOTE: I added this
    if (food.active) {
        DrawSprite(foodSprite);
    }

	glutSwapBuffers();
}

void Reshape(int h, int v)
{
	glViewport(0, 0, h, v);
	gWidth = h;
	gHeight = v;
}

// Example of user controllable parameter
float someValue = 0.0;

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
    	someValue -= 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case 0x1b:
      exit(0);
  }
}

// NOTE: I added this
void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        SpawnFood((float)x, (float)y);

        if (foodSprite == NULL) {
            foodSprite = NewSprite(foodFace, x, y, 0, 0);
            foodSprite->isFood = 1;
        } else {
            foodSprite->position = food.position;
        }
    }
}

void Init()
{
	TextureData *sheepFace, *blackieFace;
	
	LoadTGATextureSimple("bilder/leaves.tga", &backgroundTexID); // Background
	
	sheepFace = GetFace("bilder/sheep.tga"); // A sheep
	blackieFace = GetFace("bilder/blackie.tga"); // A black sheep
	foodFace = GetFace("bilder/mat.tga"); // Food
	
	//NewSprite(sheepFace, 100, 200, 1, 1);
	//NewSprite(sheepFace, 200, 100, 1.5, -1);
	//NewSprite(sheepFace, 250, 200, -1, 1.5);

	// NOTE: I added this
    for (int i = 0; i < 10; i++) {
        NewSprite(sheepFace, rand() % 800, rand() % 600, ((rand() % 100) / 50.0f) - 1, ((rand() % 100) / 50.0f) - 1);
    }

    SpritePtr blackSheep = NewSprite(blackieFace, rand() % 800, rand() % 600, ((rand() % 100) / 50.0f) - 1, ((rand() % 100) / 50.0f) - 1);
    blackSheep->isBlackSheep = 1;
    blackSheep->isFood = 0;

    food.active = 0;
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

    // NOTE: I added this
    glutMouseFunc(Mouse);
	
	InitSpriteLight();
	Init();
	
	glutMainLoop();
	return 0;
}
