#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <queue>
#include "Util.h"


#define NUM_FLOORS       8
#define LIFT_SPEED       0.4f
#define PERSON_SPEED     0.6f
#define DOOR_OPEN_TIME   5.0f
#define DOOR_ANIM_SPEED  0.6f

enum class ElevatorState {
    Idle,
    Moving,
    DoorsOpen,
    Stopped
};

struct Elevator {
    float x;
    float y;
    float width;
    float height;

    int currentFloor;
    int targetFloor;
    std::queue<int> queue;

    ElevatorState state;
    bool doorsOpen;
    double doorOpenStart;

    float doorLeftOffset;
    float doorRightOffset;
    bool doorOpening;
    bool doorClosing;

    bool openExtended; 

    Elevator()
        : x(0.8f), y(0.0f), width(0.15f), height(0.18f),
        currentFloor(1), targetFloor(1),
        state(ElevatorState::Idle),
        doorsOpen(false), doorOpenStart(-1.0),
        doorLeftOffset(0.0f), doorRightOffset(0.0f),
        doorOpening(false), doorClosing(false),
        openExtended(false)
    {
    }
};

struct Person {
    float x, y;
    float width, height;
    bool inElevator;
    int floor;

    Person()
        : x(0.3f), y(-0.75f),
        width(0.10f), height(0.16f),   
        inElevator(false), floor(0)
    {
    }
};

struct Button {
    float cx = 0, cy = 0;
    float sx = 0, sy = 0;
    unsigned int texture = 0;
    bool active = false;
};


extern GLFWwindow* window;
extern int winWidth, winHeight;

extern unsigned int rectShader, shaderTex;
extern unsigned int VAOrect, VAOtex;

extern unsigned int tex_SU, tex_PR, tex_1, tex_2, tex_3, tex_4, tex_5, tex_6;
extern unsigned int tex_open, tex_close, tex_stop, tex_vent;
extern unsigned int tex_person;     
extern unsigned int tex_signature; 

extern Elevator gElevator;
extern Person  gPerson;

extern float  gFloorY[NUM_FLOORS];
extern double gLastTime;

extern Button btnSU, btnPR;
extern Button btnFloor[6];
extern Button btnOpen, btnClose, btnStop, btnVent;

extern bool        ventActive;
extern GLFWcursor* gPropellerCursor;



float floorIndexToY(int logicalFloor);

void setupRectVAO();
void setupTextureVAO();

void initButtons();

void drawRect(float cx, float cy, float sx, float sy, float r, float g, float b);
void drawButton(unsigned int tex, float cx, float cy, float w, float h);
void drawButtonWithOutline(Button& b);
void drawScene();

void addElevatorTarget(int floorIndex);
void updateElevator(float dt, double timeNow);
void updatePerson(float dt);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
