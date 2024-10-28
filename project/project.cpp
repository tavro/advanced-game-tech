#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "MicroGlut.h"
#define MAIN
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "GL_utilities.h"
#include "LoadTGA.h"

/* PROJECT SPECIFIC CODE */
#include <iostream>
#include <vector>
#include <cmath>

struct Point {
    double x, y;
};

typedef struct
{
  Model* model;
  GLuint textureId;
} ModelTexturePair;

typedef struct
{
  GLuint tex;
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
  GLuint tex;
  vec3 P; // Position
  mat4 R; // Rotation
  mat4 targetR;
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

Model *speaker;
Model *penguin;

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
    scaleMatrix = S(0.5, 0.5, 0.5);
    glBindTexture(GL_TEXTURE_2D, player.tex);
    transMatrix = T(player.P.x, 0.1, player.P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * player.R * scaleMatrix; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    glUniform1i(glGetUniformLocation(shader, "isTex"), 1);
    DrawModel(penguin, shader, "in_Position", NULL, "in_TexCoord");
}

void renderSpeaker(int index)
{
    glBindTexture(GL_TEXTURE_2D, speakers[index].tex);
    transMatrix = T(speakers[index].P.x, 0.1, speakers[index].P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * speakers[index].R * scaleMatrix; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    glUniform1i(glGetUniformLocation(shader, "isTex"), 1);
    DrawModel(speaker, shader, "in_Position", NULL, "in_TexCoord");
}

void renderFloor(int index, float y)
{
    scaleMatrix = S(0.5, 0.5, 0.5);
    transMatrix = T(floor_array[index].P.x, y, floor_array[index].P.z);
    tmpMatrix = modelToWorldMatrix * transMatrix * floor_array[index].R * scaleMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    glUniform3fv(glGetUniformLocation(shader, "floorColor"), 1, &floor_array[index].color.x);
    glUniform1i(glGetUniformLocation(shader, "isTex"), 0);
    DrawModel(cube, shader, "in_Position", NULL, "in_TexCoord");
}

mat4 rotationFromTo(const vec3& from, const vec3& to) {
    vec3 f = Normalize(from);
    vec3 t = Normalize(to);
    vec3 axis = cross(f, t);
    float angle = std::acos(dot(f, t));
    
    mat4 rotationMatrix = ArbRotate(axis, angle);
    
    return rotationMatrix;
}

mat4 yAxisRotationFromTo(const vec3& from, const vec3& to) {
    vec3 fromXZ = Normalize(vec3(from.x, 0, from.z));
    vec3 toXZ = Normalize(vec3(to.x, 0, to.z));
    
    float angle = std::acos(dot(fromXZ, toXZ));

    vec3 crossProd = cross(fromXZ, toXZ);
    if (crossProd.y < 0) angle = -angle;

    return ArbRotate(vec3(0, 1, 0), angle);
}

struct Quaternion {
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    Quaternion(const mat4& m) {
        float trace = m.m[0] + m.m[5] + m.m[10];
        if (trace > 0.0f) {
            float s = 0.5f / sqrtf(trace + 1.0f);
            w = 0.25f / s;
            x = (m.m[9] - m.m[6]) * s;
            y = (m.m[2] - m.m[8]) * s;
            z = (m.m[4] - m.m[1]) * s;
        } else {
            if (m.m[0] > m.m[5] && m.m[0] > m.m[10]) {
                float s = 2.0f * sqrtf(1.0f + m.m[0] - m.m[5] - m.m[10]);
                w = (m.m[9] - m.m[6]) / s;
                x = 0.25f * s;
                y = (m.m[1] + m.m[4]) / s;
                z = (m.m[2] + m.m[8]) / s;
            } else if (m.m[5] > m.m[10]) {
                float s = 2.0f * sqrtf(1.0f + m.m[5] - m.m[0] - m.m[10]);
                w = (m.m[2] - m.m[8]) / s;
                x = (m.m[1] + m.m[4]) / s;
                y = 0.25f * s;
                z = (m.m[6] + m.m[9]) / s;
            } else {
                float s = 2.0f * sqrtf(1.0f + m.m[10] - m.m[0] - m.m[5]);
                w = (m.m[4] - m.m[1]) / s;
                x = (m.m[2] + m.m[8]) / s;
                y = (m.m[6] + m.m[9]) / s;
                z = 0.25f * s;
            }
        }
    }

    mat4 toMat4() const {
        mat4 result = IdentityMatrix();
        result.m[0] = 1 - 2 * (y * y + z * z);
        result.m[1] = 2 * (x * y - z * w);
        result.m[2] = 2 * (x * z + y * w);
        result.m[4] = 2 * (x * y + z * w);
        result.m[5] = 1 - 2 * (x * x + z * z);
        result.m[6] = 2 * (y * z - x * w);
        result.m[8] = 2 * (x * z - y * w);
        result.m[9] = 2 * (y * z + x * w);
        result.m[10] = 1 - 2 * (x * x + y * y);
        return result;
    }

    static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) {
        float dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

        Quaternion q2Adjusted = dot < 0.0f ? Quaternion(-q2.w, -q2.x, -q2.y, -q2.z) : q2;

        dot = fabs(dot);
        float theta = acosf(dot);
        float sinTheta = sinf(theta);

        if (sinTheta > 1e-5) {
            float scale0 = sinf((1 - t) * theta) / sinTheta;
            float scale1 = sinf(t * theta) / sinTheta;
            return Quaternion(
                scale0 * q1.w + scale1 * q2Adjusted.w,
                scale0 * q1.x + scale1 * q2Adjusted.x,
                scale0 * q1.y + scale1 * q2Adjusted.y,
                scale0 * q1.z + scale1 * q2Adjusted.z
            );
        } else {
            return Quaternion(
                q1.w * (1 - t) + q2Adjusted.w * t,
                q1.x * (1 - t) + q2Adjusted.x * t,
                q1.y * (1 - t) + q2Adjusted.y * t,
                q1.z * (1 - t) + q2Adjusted.z * t
            );
        }
    }
};

mat4 slerpRotation(const mat4& from, const mat4& to, float factor) {
    Quaternion qFrom(from);
    Quaternion qTo(to);
    Quaternion qInterpolated = Quaternion::Slerp(qFrom, qTo, factor);
    return qInterpolated.toMat4();
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

    speaker = LoadModelPlus("custom-models/Speaker.obj");
    cube = LoadModelPlus("cube.obj");
    penguin = LoadModelPlus("custom-models/Penguin_Body.obj");

    projectionMatrix = perspective(90, 1.0, 0.1, 1000);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

    modelToWorldMatrix = IdentityMatrix();

    point = vec3(0, 0, 1);

    // Initialize speakers
    player.P = vec3(0, 10, 0);
    
    player.R = IdentityMatrix();
    player.targetR = IdentityMatrix();

    player.v = vec3(0, 0, 0);

    char *textureStr = (char *)malloc(128);
    sprintf(textureStr, "custom-models/Speaker_Albedo.tga");
    std::vector<Point> points = generatePoints({0.0, 0.0}, speakerRadius);
    for (int i = 0; i < kNumSpeakers; i++)
    {
        Point p = points[i];
        speakers[i].P = vec3(p.x, 10, p.y);

        vec3 target(0.0f, 10.0f, 0.0f);
        vec3 speakerPos = speakers[i].P;
        vec3 directionToTarget = normalize(target - speakerPos);
        vec3 forwardDirection(0.0f, 0.0f, 1.0f);
        speakers[i].R = rotationFromTo(forwardDirection, directionToTarget);
        if (i == 2) {
            speakers[i].R = ArbRotate(vec3(0.0f, 1.0f, 0.0f), M_PI);
        }

        LoadTGATextureSimple(textureStr, &speakers[i].tex);
    }
    sprintf(textureStr, "custom-models/Penguin_Albedo.tga");
    LoadTGATextureSimple(textureStr, &player.tex);

	free(textureStr);

    generateFloor({0.0, 0.0});

    cam = vec3(0, 2.2, 3.5);
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

        vec3 forwardDirection = vec3(0, 0, 1);
        player.targetR = yAxisRotationFromTo(forwardDirection, direction);

        direction = ScalarMult(direction, -1);
        float forceMagnitude = 1.0f;
        player.v = ScalarMult(direction, forceMagnitude);

        nextBeatTime += beatInterval;
    }

    float rotationSpeed = 0.05f;
    player.R = slerpRotation(player.R, player.targetR, rotationSpeed);

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

    printError("uploading to shader");

    for (int i = 0; i < kNumSpeakers; i++) {
        if (isSpeakerScaled && i == scaledSpeaker) {
            float elapsedTime = currentTime - scaleStartTime;
            if (elapsedTime < scaleDuration) {
                float pulseFactor = 1.0f + 0.3f * sin((elapsedTime / scaleDuration) * M_PI);
                scaleMatrix = S(pulseFactor, pulseFactor, pulseFactor);
            } else {
                isSpeakerScaled = false;
                scaleMatrix = S(0.5, 0.5, 0.5);
            }
        } else {
            scaleMatrix = S(0.5, 0.5, 0.5);
        }

        renderSpeaker(i);
    }

    for (int i = 0; i < kNumFloor * kNumFloor; i++) {
        renderFloor(i, -0.5);
    }

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