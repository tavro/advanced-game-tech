#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "MicroGlut.h"
#define MAIN
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "GL_utilities.h"

/* PROJECT SPECIFIC CODE */
#include <iostream>
#include <vector>
#include <cmath>

struct Point {
    double x, y;
};

typedef struct
{
  // TODO: GLuint tex;
  vec3 P; // Position
  mat4 R; // Rotation
} Speaker;

typedef struct
{
  // TODO: GLuint tex;
  vec3 P; // Position
  mat4 R; // Rotation
  vec3 color;
} Floor;

typedef struct
{
  // TODO: GLuint tex;
  vec3 P; // Position
  mat4 R; // Rotation
  vec3 v; // Velocity
} Player;

// === GLOBALS ===
float speakerRadius = 2.0f;
const int initWidth = 800, initHeight = 800;

FBOstruct *fbo1;

GLuint shader = 0;
GLfloat deltaT, currentTime;

vec3 cam, point;

enum {kNumSpeakers = 8};
enum {kNumFloor = 4};

Model *sphere; // TODO: This should be the speaker model later
Model *cube;

Floor floor_array[999];
Speaker speakers[16];
Player player;

mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;
mat4 rotateMatrix, scaleMatrix, transMatrix, tmpMatrix;

/* BEAT STUFF */
int bpm = 140;
float beatInterval;
float nextBeatTime;
int scaledSpeaker = -1;
bool isSpeakerScaled = false;
float scaleDuration = 0.2;
float scaleStartTime = 0.0;
// ===============

std::vector<Point> generatePoints(Point center, double radius) {
    /* USED FOR POSITIONING THE SPEAKERS */
    std::vector<Point> points;
    double angleIncrement = 2 * M_PI / kNumSpeakers;

    for (int i = 0; i < kNumSpeakers; ++i) {
        double angle = i * angleIncrement;
        Point point;
        point.x = center.x + radius * cos(angle);
        point.y = center.y + radius * sin(angle);
        points.push_back(point);
    }

    return points;
}

void generateFloor(Point center) {
    float half_wh = (kNumFloor - 1) / 2.0f;
    int index = 0;

    for (int row = 0; row < kNumFloor; row++) {
        for (int col = 0; col < kNumFloor; col++) {
            float x = center.x + (col - half_wh);
            float z = center.y + (row - half_wh);

            floor_array[index].P = (vec3){x, 0.0f, z};
            floor_array[index].R = IdentityMatrix();

            floor_array[index].color = (vec3){(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX};

            index++;

            if (index >= 999) {
                return;
            }
        }
    }
}

void renderPlayer()
{
    transMatrix = T(player.P.x, 0.1, player.P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * player.R * scaleMatrix; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    DrawModel(sphere, shader, "in_Position", NULL, NULL);
}

void renderSpeaker(int index)
{
    // glBindTexture(GL_TEXTURE_2D, speakers[index].tex);
    transMatrix = T(speakers[index].P.x, 0.1, speakers[index].P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * speakers[index].R * scaleMatrix; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    DrawModel(sphere, shader, "in_Position", NULL, NULL);
}

void renderFloor(int index, float y)
{
    transMatrix = T(floor_array[index].P.x, y, floor_array[index].P.z);
    tmpMatrix = modelToWorldMatrix * transMatrix * floor_array[index].R * scaleMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    glUniform3fv(glGetUniformLocation(shader, "floorColor"), 1, &floor_array[index].color.x);
    DrawModel(cube, shader, "in_Position", NULL, NULL);
}
/* ========== */

void init()
{
    dumpInfo();

    // GL inits
    glClearDepth(1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    printError("GL inits");

    // Load shader
    shader = loadShaders("temp.vert", "temp.frag");
    printError("init shader");

    sphere = LoadModelPlus("sphere.obj");
    cube = LoadModelPlus("cube.obj");

    projectionMatrix = perspective(90, 1.0, 0.1, 1000);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

    modelToWorldMatrix = IdentityMatrix();

    point = vec3(0, 0, 1);

    // Initialize speakers
    player.P = vec3(0, 10, 0);
    player.R = IdentityMatrix();
    player.v = vec3(0, 0, 0);
    std::vector<Point> points = generatePoints({0.0, 0.0}, speakerRadius);
    for (int i = 0; i < kNumSpeakers; i++)
    {
        Point p = points[i];
        speakers[i].P = vec3(p.x, 10, p.y);
        speakers[i].R = IdentityMatrix();
    }

    generateFloor({0.0, 0.0});

    cam = vec3(0, 1.2, 2.5);
    viewMatrix = lookAtv(cam, point, vec3(0, 1, 0));

    beatInterval = 60.0f / bpm;
    nextBeatTime = beatInterval;
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
    deltaT = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - currentTime;
    currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

    if (currentTime >= nextBeatTime) {
        if (isSpeakerScaled) {
            isSpeakerScaled = false;
        }

        scaledSpeaker = rand() % kNumSpeakers;
        isSpeakerScaled = true;
        scaleStartTime = currentTime;

        vec3 direction = VectorSub(speakers[scaledSpeaker].P, player.P);

        direction = Normalize(direction);
        direction = ScalarMult(direction, -1);

        float forceMagnitude = 1.0f;
        player.v = ScalarMult(direction, forceMagnitude);

        nextBeatTime += beatInterval;
    }

    player.P = VectorAdd(player.P, ScalarMult(player.v, deltaT));

    player.v = ScalarMult(player.v, 0.9);

    float dist = sqrt(pow(player.P.x, 2) + pow(player.P.y - 10, 2) + pow(player.P.z, 2));
    if (dist > speakerRadius) {
        player.P = vec3(0, 10, 0);
    }

    glClearColor(0.4, 0.5, 0.9, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, viewMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);


    vec3 color = (vec3){1, 0, 0};
    glUniform3fv(glGetUniformLocation(shader, "floorColor"), 1, &color.x);

    printError("uploading to shader");

    for (int i = 0; i < kNumSpeakers; i++) {
        if (isSpeakerScaled && i == scaledSpeaker) {
            if (currentTime - scaleStartTime < scaleDuration) {
                scaleMatrix = S(1.5, 1.5, 1.5);
            } else {
                isSpeakerScaled = false;
                scaleMatrix = S(1.0, 1.0, 1.0);
            }
        } else {
            scaleMatrix = S(1.0, 1.0, 1.0);
        }

        renderSpeaker(i);
    }

    scaleMatrix = S(0.5, 0.5, 0.5);

    for (int i = 0; i < kNumFloor*kNumFloor; i++) {
        renderFloor(i, -0.5);
    }

    glUniform3fv(glGetUniformLocation(shader, "floorColor"), 1, &color.x);
    renderPlayer();

    printError("rendering");

    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	GLfloat ratio = (GLfloat) w / (GLfloat) h;
	projectionMatrix = perspective(90, ratio, 1.0, 1000);
}

int prevx = 0, prevy = 0;
void mouseUpDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		prevx = x;
		prevy = y;
	}
}

void mouseDragged(int x, int y)
{
	vec3 p;
	mat4 m;
	
	p.y = x - prevx;
	p.x = -(prevy - y);
	p.z = 0;

	m = ArbRotate(p, sqrt(p.x*p.x + p.y*p.y) / 50.0);
	modelToWorldMatrix = Mult(m, modelToWorldMatrix);
	
	prevx = x;
	prevy = y;
	
	glutPostRedisplay();
}


//-----------------------------main-----------------------------------------------
int main(int argc, char *argv[])
{
    srand(time(NULL));
    
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(initWidth, initHeight);

	glutInitContextVersion(3, 2);
	glutCreateWindow ("Project");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	glutRepeatingTimer(50);

	init();
	glutMainLoop();
	exit(0);
}