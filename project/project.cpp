#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "MicroGlut.h"
#define MAIN
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "GL_utilities.h"
#include "LoadTGA.h"

/*

TODO
====

- count down sound + dropping bird to ground
- visual feedback (sound waves, 3... 2... 1...), adjust map
- animation when falling of dance floor
- flapping wings
- improve flocking behavior

*/

/* PROJECT SPECIFIC CODE */
#include <iostream>
#include <vector>
#include <cmath>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_engine engine;

struct Point {
    double x, y;
};

typedef struct {
    int bpm;
    float duration;
    char* path;
} MusicData;

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
  bool isActive;
  bool scaling;
  float scaleStartTime;
  float scaleDuration;
} Speaker;

typedef struct
{
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

enum {kNumAudience = 16};
enum {kNumSpeakers = 8};
enum {kNumFloor = 4};

Model *speaker;
Model *penguin;

Model *cube;

Floor floor_array[999];
Player audience[999];
Speaker speakers[16];
MusicData songs[3];
Player player;

mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;
mat4 rotateMatrix, scaleMatrix, transMatrix, tmpMatrix;

/* BEAT STUFF */
int bpm = 140;
float beatInterval;
float nextBeatTime;
int scaledSpeaker = -1;

std::vector<Point> generateAudiencePoints(Point center, double radius, double distance) {
    std::vector<Point> points;

    std::srand(time(nullptr));

    for (int i = 0; i < kNumAudience; ++i) {
        double angle = ((double)std::rand() / RAND_MAX) * 2.0 * M_PI;

        double randRadius = radius + ((double)std::rand() / RAND_MAX) * distance;

        double x = center.x + randRadius * std::cos(angle);
        double y = center.y + randRadius * std::sin(angle);

        points.push_back({x, y});
    }

    return points;
}

float length(const vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

mat4 rotationFromTo(const vec3& from, const vec3& to) {
    vec3 f = Normalize(from);
    vec3 t = Normalize(to);
    vec3 axis = cross(f, t);
    float angle = std::acos(dot(f, t));
    
    mat4 rotationMatrix = ArbRotate(axis, angle);
    
    return rotationMatrix;
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

Point targetPoint;
void moveTowardsPoint(int index) {
    Point center = {0.0f, 0.0f};
    double radius = speakerRadius + 1.0;
    double maxRadius = radius + 1.0;

    // Constants for flocking behavior
    const float separationDistance = 0.5f;
    const float maxVelocity = 0.1f;
    const float cohesionWeight = 0.01f;
    const float separationWeight = 0.02f;
    const float alignmentWeight = 0.005f;
    const float rotationSpeed = 0.05f;

    Player& currentPenguin = audience[index];
    vec3 newVelocity(0.0f, 0.0f, 0.0f);

    // SEPARATION
    for (int i = 0; i < kNumAudience; ++i) {
        if (i == index) continue;

        vec3 toOther = audience[i].P - currentPenguin.P;
        float distanceToOther = length(toOther);

        if (distanceToOther < separationDistance) {
            newVelocity -= normalize(toOther) * separationWeight;
        }
    }

    // COHESION
    vec3 targetVec(targetPoint.x, 0.0f, targetPoint.y);
    vec3 toTarget = targetVec - currentPenguin.P;
    newVelocity += normalize(toTarget) * cohesionWeight;

    // ALIGNMENT
    vec3 averageVelocity(0.0f, 0.0f, 0.0f);
    int neighborCount = 0;
    for (int i = 0; i < kNumAudience; ++i) {
        if (i == index) continue;

        vec3 toOther = audience[i].P - currentPenguin.P;
        if (length(toOther) < separationDistance * 2.0f) {
            averageVelocity += audience[i].v;
            neighborCount++;
        }
    }
    if (neighborCount > 0) {
        averageVelocity /= static_cast<float>(neighborCount);
        newVelocity += normalize(averageVelocity) * alignmentWeight;
    }

    if (length(newVelocity) > maxVelocity) {
        newVelocity = normalize(newVelocity) * maxVelocity;
    }

    currentPenguin.v = newVelocity;
    currentPenguin.P += currentPenguin.v;

    vec3 toCenter = currentPenguin.P - vec3(center.x, 0.0f, center.y);
    float currentDistance = length(toCenter);

    if (currentDistance < radius) {
        currentPenguin.P = vec3(center.x, 0.0f, center.y) + normalize(toCenter) * static_cast<float>(radius);
    } else if (currentDistance > maxRadius) {
        currentPenguin.P = vec3(center.x, 0.0f, center.y) + normalize(toCenter) * static_cast<float>(maxRadius);
    }

    if (length(currentPenguin.P - targetVec) < 1.0f) {
        double randAngle = ((double)std::rand() / RAND_MAX) * 2.0 * M_PI;

        double randRadius = radius + ((double)std::rand() / RAND_MAX);

        targetPoint.x = center.x + randRadius * std::cos(randAngle);
        targetPoint.y = center.y + randRadius * std::sin(randAngle);
    }

    if (length(currentPenguin.v) > 0.0f) {
        vec3 forwardDirection(0.0f, 0.0f, 1.0f);
        vec3 newForward = normalize(currentPenguin.v);

        mat4 targetRotation = rotationFromTo(forwardDirection, newForward);

        currentPenguin.R = slerpRotation(currentPenguin.R, targetRotation, rotationSpeed);
    }
}

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

void renderAudience(int index) {
    scaleMatrix = S(0.5, 0.5, 0.5);
    glBindTexture(GL_TEXTURE_2D, audience[index].tex);
    transMatrix = T(audience[index].P.x, -0.5, audience[index].P.z); // position
    tmpMatrix = modelToWorldMatrix * transMatrix * audience[index].R * scaleMatrix; // rotation
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, tmpMatrix.m);
    glUniform1i(glGetUniformLocation(shader, "isTex"), 1);
    DrawModel(penguin, shader, "in_Position", NULL, "in_TexCoord");
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

mat4 yAxisRotationFromTo(const vec3& from, const vec3& to) {
    vec3 fromXZ = Normalize(vec3(from.x, 0, from.z));
    vec3 toXZ = Normalize(vec3(to.x, 0, to.z));
    
    float angle = std::acos(dot(fromXZ, toXZ));

    vec3 crossProd = cross(fromXZ, toXZ);
    if (crossProd.y < 0) angle = -angle;

    return ArbRotate(vec3(0, 1, 0), angle);
}

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

    player.P = vec3(0, 10, 0);
    
    player.R = IdentityMatrix();
    player.targetR = IdentityMatrix();

    player.v = vec3(0, 0, 0);

    // Initialize speakers
    char *textureStr = (char *)malloc(128);
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
        
        sprintf(textureStr, "custom-models/Speaker_Albedo_%d.tga", i + 1);

        LoadTGATextureSimple(textureStr, &speakers[i].tex);

        speakers[i].isActive = false;
        speakers[i].scaling = false;
        speakers[i].scaleStartTime = 0.0f;
        speakers[i].scaleDuration = 0.1f;
    }
    // Initialize audience
    sprintf(textureStr, "custom-models/Penguin_Albedo.tga");
    std::vector<Point> audiencePoints = generateAudiencePoints({0.0, 0.0}, speakerRadius + 1.0, 2.0);
    for (int i = 0; i < kNumAudience; i++) {
        Point p = audiencePoints[i];
        audience[i].P = vec3(p.x, 0, p.y);

        vec3 target(0.0f, 0.0f, 0.0f);
        vec3 audiencePos = audience[i].P;
        vec3 directionToTarget = normalize(target - audiencePos);
        vec3 forwardDirection(0.0f, 0.0f, 1.0f);
        audience[i].R = rotationFromTo(forwardDirection, directionToTarget);

        LoadTGATextureSimple(textureStr, &audience[i].tex);
    }
    targetPoint = audiencePoints[0];
    LoadTGATextureSimple(textureStr, &player.tex);

	free(textureStr);

    generateFloor({0.0, 0.0});

    cam = vec3(0, 3.2, 3.5);
    viewMatrix = lookAtv(cam, point, vec3(0, 1, 0));

    beatInterval = 60.0f / bpm;
    nextBeatTime = beatInterval;
}

//-------------------------------callback functions------------------------------------------
bool currentlyPlaying = false;
float secondsPassed = 0.0f;
int activeIndex = 0;
void display(void)
{
    deltaT = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - currentTime;
    currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    
    secondsPassed += deltaT;
    if(secondsPassed > songs[activeIndex].duration) {
        secondsPassed = 0.0f;
        currentlyPlaying = false;
        activeIndex++;
        if(activeIndex > 2) {
            activeIndex = 0;
        }
        /*
        TODO
        beatInterval = 60.0f / songs[activeIndex].bpm;
        nextBeatTime = beatInterval;
        */
    }

    if(!currentlyPlaying) {
        ma_engine_play_sound(&engine, songs[activeIndex].path, NULL);
        currentlyPlaying = true;
    }

    if (currentTime >= nextBeatTime) {
        scaledSpeaker = rand() % kNumSpeakers;
        speakers[scaledSpeaker].isActive = true;
        nextBeatTime += beatInterval;
    }

    float rotationSpeed = 0.05f;
    player.R = slerpRotation(player.R, player.targetR, rotationSpeed);

    player.P = VectorAdd(player.P, ScalarMult(player.v, deltaT));
    player.v = ScalarMult(player.v, 0.9);

    float dist = sqrt(pow(player.P.x, 2) + pow(player.P.y - 10, 2) + pow(player.P.z, 2));
    if (dist > speakerRadius) {
        player.P = vec3(0, 10, 0);

        ma_engine_uninit(&engine);
        currentlyPlaying = false;
        secondsPassed = 0.0f;
        activeIndex = 0;
        ma_engine_init(NULL, &engine);
        player.v = 0;
    }

    glClearColor(0.4, 0.4, 0.4, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, viewMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelToWorldMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);

    printError("uploading to shader");

    vec3 totalForce = vec3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < kNumSpeakers; i++)
    {
        if (speakers[i].isActive)
        {
            speakers[i].scaling = true;
            speakers[i].scaleStartTime = currentTime;

            vec3 direction = VectorSub(speakers[i].P, player.P);
            direction = Normalize(direction);
                    
            vec3 forwardDirection = vec3(0, 0, 1);
            player.targetR = yAxisRotationFromTo(forwardDirection, direction);

            direction = ScalarMult(direction, -1);
            float forceMagnitude = 1.0f;

            totalForce = VectorAdd(totalForce, ScalarMult(direction, forceMagnitude));

            speakers[i].isActive = false;
        }

        if (speakers[i].scaling)
        {
            float scaleTime = currentTime - speakers[i].scaleStartTime;
            float scaleFactor = 0.5f;

            if (scaleTime < speakers[i].scaleDuration) {
                scaleFactor = 0.5f + (0.5f * (scaleTime / speakers[i].scaleDuration));
            } else if (scaleTime < 2 * speakers[i].scaleDuration) {
                scaleFactor = 1.0f - (0.5f * ((scaleTime - speakers[i].scaleDuration) / speakers[i].scaleDuration));
            } else {
                speakers[i].scaling = false;
                scaleFactor = 0.5f;
            }

            scaleMatrix = S(scaleFactor, scaleFactor, scaleFactor);
        } else {
            scaleMatrix = S(0.5, 0.5, 0.5);
        }

        renderSpeaker(i);
    }

    for (int i = 0; i < kNumAudience; i++) {
        renderAudience(i);
        moveTowardsPoint(i);
    }

    player.v = VectorAdd(player.v, totalForce);

    scaleMatrix = S(0.5, 0.5, 0.5);
    for (int i = 0; i < kNumFloor * kNumFloor; i++) {
        renderFloor(i, -0.5);
    }

    scaleMatrix = S(0.5, 0.5, 0.5);
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

void keyPress(unsigned char key, int x, int y)
{
    if (key >= '1' && key <= '8')
    {
        int speakerIndex = key - '1';
        if (speakerIndex < kNumSpeakers)
        {
            speakers[speakerIndex].isActive = true;
        }
    }
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
    glutKeyboardFunc(keyPress);
	glutRepeatingTimer(50);

	init();

    ma_result result;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        return -1;
    }
    
    songs[0].bpm = 140;
    songs[0].duration = 29;
    songs[0].path = "audio/140_icy_groove.mp3";
    songs[1].bpm = 150;
    songs[1].duration = 40;
    songs[1].path = "audio/150_antarctic_bass.mp3";
    songs[2].bpm = 160;
    songs[2].duration = 57;
    songs[2].path = "audio/160_frozen_beats.mp3";

	glutMainLoop();

    ma_engine_uninit(&engine);
	exit(0);
}