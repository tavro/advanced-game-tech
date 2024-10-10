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

// === GLOBALS ===
const int initWidth = 800, initHeight = 800;

FBOstruct *fbo1;

GLuint shader = 0;
GLfloat deltaT, currentTime;

vec3 cam, point;

enum {kNumSpeakers = 8};

Model *sphere; // TODO: This should be the speaker model later

Speaker speakers[16];

mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;
mat4 rotateMatrix, scaleMatrix, transMatrix, tmpMatrix;
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

void renderSpeaker(int index)
{
    // glBindTexture(GL_TEXTURE_2D, speakers[index].tex);

    // NOTE: 0.1 is size
    transMatrix = T(speakers[index].P.x, 0.1, speakers[index].P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * speakers[index].R; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    DrawModel(sphere, shader, "in_Position", NULL, NULL);
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

    projectionMatrix = perspective(90, 1.0, 0.1, 1000);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

	modelToWorldMatrix = IdentityMatrix();

    point = vec3(0, 0, 1);

    // Initialize speakers
    std::vector<Point> points = generatePoints({0.0, 0.0}, 1.0f);
	for (int i = 0; i < kNumSpeakers; i++)
	{
        Point p = points[i];
		speakers[i].P = vec3(p.x, 10, p.y);
		speakers[i].R = IdentityMatrix();
	}

    cam = vec3(0, 1.2, 2.5);
    viewMatrix = lookAtv(cam, point, vec3(0, 1, 0));
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
	deltaT = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - currentTime;
	currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	
	int i;

	glClearColor(0.4, 0.5, 0.9, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, viewMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
    
    printError("uploading to shader");

	for (i = 0; i < kNumSpeakers; i++) {
        renderSpeaker(i);
    }

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