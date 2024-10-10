#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "MicroGlut.h"
#define MAIN
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "GL_utilities.h"

const int initWidth = 800, initHeight = 800;

mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;

GLfloat square[] = {
							-1,-1,0,
							-1,1, 0,
							1,1, 0,
							1,-1, 0};
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0};
GLuint squareIndices[] = {0, 1, 2, 0, 2, 3};

Model* squareModel;

//----------------------Globals-------------------------------------------------
FBOstruct *fbo1;
GLuint shader = 0;
//-------------------------------------------------------------------------------------

void init(void)
{
	dumpInfo();

	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Load and compile shaders
	shader = loadShaders("temp.vert", "temp.frag");

	printError("init shader");

	fbo1 = initFBO(initWidth, initHeight, 0);

	vec3 cam = vec3(0, 5, 15);
	vec3 point = vec3(0, 1, 0);
	vec3 up = vec3(0, 1, 0);
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
	useFBO(fbo1, 0L, 0L);

    // Clear framebuffer & zbuffer
	glClearColor(0.2, 0.2, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(shader);

    mat4 vm2;
    vm2 = viewMatrix * modelToWorldMatrix;
	vm2 = vm2 * T(0, -8.5, 0);
	vm2 = vm2 * S(80, 80, 80);

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