#include "../Header/LiftApp.h"


GLFWwindow* window = nullptr;
int winWidth = 1280;
int winHeight = 720;

unsigned int rectShader, shaderTex;
unsigned int VAOrect, VAOtex;

unsigned int tex_SU, tex_PR, tex_1, tex_2, tex_3, tex_4, tex_5, tex_6;
unsigned int tex_open, tex_close, tex_stop, tex_vent;
unsigned int tex_person;
unsigned int tex_signature;   

Elevator gElevator;
Person   gPerson;

float  gFloorY[NUM_FLOORS];
double gLastTime = 0.0;

Button btnSU, btnPR, btnOpen, btnClose, btnStop, btnVent;
Button btnFloor[6];

bool        ventActive = false;
GLFWcursor* gPropellerCursor = nullptr;



int main()
{
    if (!glfwInit()) return endProgram("GLFW init failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    winWidth = mode->width;
    winHeight = mode->height;

    window = glfwCreateWindow(winWidth, winHeight, "Lift projekat", monitor, nullptr);
    if (!window) return endProgram("Window failed");

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glViewport(0, 0, winWidth, winHeight);
    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    rectShader = createShader("Shaders/lift.vert", "Shaders/lift.frag");
    shaderTex = createShader("Shaders/texture.vert", "Shaders/texture.frag");

    setupRectVAO();
    setupTextureVAO();

    tex_SU = loadTexture("Textures/SU.png");
    tex_PR = loadTexture("Textures/PR.png");
    tex_1 = loadTexture("Textures/1.png");
    tex_2 = loadTexture("Textures/2.png");
    tex_3 = loadTexture("Textures/3.png");
    tex_4 = loadTexture("Textures/4.png");
    tex_5 = loadTexture("Textures/5.png");
    tex_6 = loadTexture("Textures/6.png");
    tex_open = loadTexture("Textures/open.png");
    tex_close = loadTexture("Textures/close.png");
    tex_stop = loadTexture("Textures/stop.png");
    tex_vent = loadTexture("Textures/vent.png");
    tex_person = loadTexture("Textures/person.png");
    tex_signature = loadTexture("Textures/signature.png");  

    initButtons();

    for (int i = 0; i < NUM_FLOORS; i++)
        gFloorY[i] = floorIndexToY(i - 1);

    gElevator.currentFloor = 1;
    gElevator.targetFloor = 1;
    gElevator.y = floorIndexToY(1);

    gPerson.floor = 0;
    gPerson.inElevator = false;
    gPerson.x = 0.3f;

    gLastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = float(now - gLastTime);
        gLastTime = now;

        if (gElevator.state != ElevatorState::Stopped)
            updateElevator(dt, now);

        updatePerson(dt);

        drawScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (gPropellerCursor)
        glfwDestroyCursor(gPropellerCursor);

    glfwTerminate();
    return 0;
}
