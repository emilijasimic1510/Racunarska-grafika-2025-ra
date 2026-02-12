#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <cfloat>
#include <cmath>
#include <algorithm>

#include <windows.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Floor.h"
#include "ElevatorSystem.h"
#include "Player.h"

int SCR_W = 1920, SCR_H = 1080;

Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = 960.0f, lastY = 540.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const float TARGET_FPS = 75.0f;
const float FRAME_TIME = 1.0f / TARGET_FPS;

bool depthTestEnabled = true;
bool cullingEnabled = false;
bool keyF1Pressed = false;
bool keyF2Pressed = false;
bool keyCPressed = false;
bool lookAroundMode = false;

bool needsRender = true;
glm::vec3 lastPlayerPos(0.0f);
glm::vec3 lastCameraFront(0.0f, 0.0f, -1.0f);
float lastElevatorY = 0.0f;
float lastDoorOffset = 0.0f;

Player player;

unsigned int tex_SU, tex_PR, tex_1, tex_2, tex_3, tex_4, tex_5, tex_6;
unsigned int tex_open, tex_close, tex_stop, tex_vent;
unsigned int tex_signature;

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

unsigned int wallVAO = 0, wallVBO = 0, wallEBO = 0;

const float WALL_MIN_X = -5.5f;
const float WALL_MAX_X = 5.5f;
const float WALL_MIN_Z = -5.5f;
const float WALL_MAX_Z = 5.5f;

const float ELEVATOR_MODEL_HEIGHT = 224.0f;
const float DESIRED_ELEVATOR_HEIGHT = 2.8f;
const float ELEVATOR_SCALE = DESIRED_ELEVATOR_HEIGHT / ELEVATOR_MODEL_HEIGHT;

const float ELEVATOR_MODEL_BOTTOM = -208.0f;
const float ELEVATOR_Y_OFFSET = -ELEVATOR_MODEL_BOTTOM * ELEVATOR_SCALE;

const float DOOR_Z_OFFSET = 0.55f;
const float DOOR_SCALE_WIDTH = 0.008f;
const float DOOR_SCALE_HEIGHT = 0.078f;

const float CABIN_HALF_WIDTH = 1.8f;
const float CABIN_HALF_DEPTH = 1.8f;

void InitQuad()
{
    if (quadVAO) return;

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,

         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void InitWalls()
{
    if (wallVAO) return;

    float vertices[] = {
        -6.0f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,
         6.0f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,
         6.0f, 3.0f, 0.0f,       0.0f, 0.0f, 1.0f,
        -6.0f, 3.0f, 0.0f,       0.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);

    glBindVertexArray(wallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

unsigned int LoadTexture(const wchar_t* filename)
{
    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;

    CoInitialize(nullptr);

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );

    if (FAILED(hr)) {
        std::cout << "Failed to create WIC factory" << std::endl;
        return 0;
    }

    hr = factory->CreateDecoderFromFilename(
        filename,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr)) {
        std::cout << "Failed to load texture: " << std::endl;
        factory->Release();
        return 0;
    }

    decoder->GetFrame(0, &frame);
    factory->CreateFormatConverter(&converter);

    converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    UINT width, height;
    converter->GetSize(&width, &height);

    std::vector<unsigned char> pixels(width * height * 4);
    converter->CopyPixels(nullptr, width * 4,
        (UINT)pixels.size(), pixels.data());

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        (int)width, (int)height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        pixels.data()
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    return tex;
}

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    camera.ProcessMouseMovement((float)xpos - lastX, lastY - (float)ypos);
    lastX = (float)xpos;
    lastY = (float)ypos;

    needsRender = true;
}

bool g_playerInElevator = false;

struct Button2D {
    float cx, cy, w, h;
    unsigned int texture;
    int targetFloor;
    enum Type { Floor, Open, Close, Stop, Vent } type;
    bool active;
};

bool MouseInRect(double mx, double my, float cx, float cy, float w, float h)
{
    return (mx > cx - w * 0.5 && mx < cx + w * 0.5 &&
        my > cy - h * 0.5 && my < cy + h * 0.5);
}

void DrawButton(Shader& shader, unsigned int texture, float cx, float cy, float w, float h, bool active)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("uTex", 0);

    glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(cx, cy, 0.0f));
    M = glm::scale(M, glm::vec3(w, h, 1.0f));

    shader.setMat4("uM", M);
    shader.setInt("uActive", active ? 1 : 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ApplyWallCollision(glm::vec3& position, bool inElevator)
{
    if (inElevator) return;

    if (position.x < WALL_MIN_X) position.x = WALL_MIN_X;
    if (position.x > WALL_MAX_X) position.x = WALL_MAX_X;
    if (position.z < WALL_MIN_Z) position.z = WALL_MIN_Z;
    if (position.z > WALL_MAX_Z) position.z = WALL_MAX_Z;
}

void SetMaterialColor(Shader& shader, glm::vec3 color, float shininess = 32.0f)
{
    shader.setVec3("uMaterial.kA", color * 0.3f);
    shader.setVec3("uMaterial.kD", color);
    shader.setVec3("uMaterial.kS", glm::vec3(0.5f));
    shader.setFloat("uMaterial.shine", shininess);
}

bool CheckIfRenderNeeded(const Player& p, const Camera& cam, const ElevatorSystem& elev)
{
    const float EPSILON = 0.001f;

    if (glm::distance(p.position, lastPlayerPos) > EPSILON) {
        lastPlayerPos = p.position;
        return true;
    }

    if (glm::distance(cam.Front, lastCameraFront) > EPSILON) {
        lastCameraFront = cam.Front;
        return true;
    }

    if (std::fabs(elev.y - lastElevatorY) > EPSILON) {
        lastElevatorY = elev.y;
        return true;
    }

    if (std::fabs(elev.doorLeftOffset - lastDoorOffset) > EPSILON) {
        lastDoorOffset = elev.doorLeftOffset;
        return true;
    }

    return false;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    SCR_W = mode->width;
    SCR_H = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "3D Elevator - Emilija Simić RA3/2022", monitor, nullptr);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();

    glViewport(0, 0, SCR_W, SCR_H);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader shader("res/shadders/phong.vert", "res/shadders/phong.frag");
    Shader buttonShader("res/shadders/panel.vert", "res/shadders/panel.frag");

    InitQuad();
    InitWalls();

    tex_SU = LoadTexture(L"res\\textures\\SU.png");
    tex_PR = LoadTexture(L"res\\textures\\PR.png");
    tex_1 = LoadTexture(L"res\\textures\\1.png");
    tex_2 = LoadTexture(L"res\\textures\\2.png");
    tex_3 = LoadTexture(L"res\\textures\\3.png");
    tex_4 = LoadTexture(L"res\\textures\\4.png");
    tex_5 = LoadTexture(L"res\\textures\\5.png");
    tex_6 = LoadTexture(L"res\\textures\\6.png");
    tex_open = LoadTexture(L"res\\textures\\open.png");
    tex_close = LoadTexture(L"res\\textures\\close.png");
    tex_stop = LoadTexture(L"res\\textures\\stop.png");
    tex_vent = LoadTexture(L"res\\textures\\vent.png");

    tex_signature = LoadTexture(L"res\\textures\\signature.png");

    Model elevatorModel("res/elevator/elevator.obj");
    Model doorModel("res/elevator/elevator-door.obj");
    Model plantAloe("res/models/plants/AloePlant.obj");
    Model plantPot("res/models/plants/potplant2.obj");
    Model plantHouse("res/models/plants/eb_house_plant_03.obj");
    Model wallLight("res/models/lights/eb_sconce_light_01.obj");

    std::vector<Floor> floors;
    for (int i = -1; i <= 6; i++)
        floors.emplace_back(glm::vec3(0.0f, FloorToWorldY(i), 0.0f), 12.0f);

    ElevatorSystem elevatorSys;
    elevatorSys.currentFloor = 0;
    elevatorSys.y = FloorToWorldY(0);

    elevatorSys.state = ElevatorState::DoorsOpen;
    elevatorSys.doorsOpen = true;
    elevatorSys.doorOpenStart = 999999.0;
    elevatorSys.doorLeftOffset = -1.5f;
    elevatorSys.doorRightOffset = 1.5f;

    player.position.y = FloorToWorldY(0) + 0.1f;
    player.position.x = -5.0f;
    player.position.z = 0.0f;

    bool mouseWasDown = false;
    bool inLiftState = false;

    std::cout << "=== 3D ELEVATOR - Emilija Simic RA3/2022 ===" << std::endl;
    std::cout << "WASD - Move" << std::endl;
    std::cout << "Mouse - Look around" << std::endl;
    std::cout << "C - Call elevator" << std::endl;
    std::cout << "KEY 1 - Toggle Depth Test" << std::endl;
    std::cout << "KEY 2 - Toggle Backface Culling" << std::endl;
    std::cout << "ESC - Exit\n" << std::endl;

    needsRender = true;

    while (!glfwWindowShouldClose(window))
    {
        double frameStart = glfwGetTime();
        float now = (float)frameStart;
        deltaTime = now - lastFrame;
        lastFrame = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        bool key1Current = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;

        if (key1Current && !keyF1Pressed) {
            depthTestEnabled = !depthTestEnabled;
            if (depthTestEnabled) {
                glEnable(GL_DEPTH_TEST);
                std::cout << "\n╔═══════════════════════════════════╗\n";
                std::cout << "║  DEPTH TEST: ON (Normal)          ║\n";
                std::cout << "║  Objects render correctly         ║\n";
                std::cout << "╚═══════════════════════════════════╝\n" << std::endl;
            }
            else {
                glDisable(GL_DEPTH_TEST);
                std::cout << "\n╔═══════════════════════════════════╗\n";
                std::cout << "║  DEPTH TEST: OFF (BROKEN!)        ║\n";
                std::cout << "║  Objects render in WRONG order!   ║\n";
                std::cout << "╚═══════════════════════════════════╝\n" << std::endl;
            }
            needsRender = true;
        }
        keyF1Pressed = key1Current;

        bool key2Current = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;

        if (key2Current && !keyF2Pressed) {
            cullingEnabled = !cullingEnabled;
            if (cullingEnabled) {
                glEnable(GL_CULL_FACE);
                std::cout << "\n╔═══════════════════════════════════╗\n";
                std::cout << "║  BACKFACE CULLING: ON             ║\n";
                std::cout << "║  Back faces not rendered          ║\n";
                std::cout << "╚═══════════════════════════════════╝\n" << std::endl;
            }
            else {
                glDisable(GL_CULL_FACE);
                std::cout << "\n╔═══════════════════════════════════╗\n";
                std::cout << "║  BACKFACE CULLING: OFF            ║\n";
                std::cout << "║  All faces rendered               ║\n";
                std::cout << "╚═══════════════════════════════════╝\n" << std::endl;
            }
            needsRender = true;
        }
        keyF2Pressed = key2Current;

        glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        bool playerMoved = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { player.position += forward * player.speed * deltaTime; playerMoved = true; }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { player.position -= forward * player.speed * deltaTime; playerMoved = true; }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { player.position -= right * player.speed * deltaTime; playerMoved = true; }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { player.position += right * player.speed * deltaTime; playerMoved = true; }

        if (playerMoved) needsRender = true;

        ApplyWallCollision(player.position, inLiftState);

        int playerFloor = std::round(player.position.y / 3.0f);
        if (playerFloor < -1) playerFloor = -1;
        if (playerFloor > 6) playerFloor = 6;

        if (!inLiftState)
            player.position.y = FloorToWorldY(playerFloor) + 0.1f;

        camera.Position = player.position + glm::vec3(0.0f, 1.6f, 0.0f);

        UpdateElevator(elevatorSys, deltaTime, now);

        glm::vec3 elevatorFloorPos(0.0f, elevatorSys.y, 0.0f);
        float dx = std::fabs(player.position.x - elevatorFloorPos.x);
        float dz = std::fabs(player.position.z - elevatorFloorPos.z);
        float playerFloorY = player.position.y - 0.1f;
        float elevatorFloorY = elevatorSys.y;
        float dy = std::fabs(playerFloorY - elevatorFloorY);

        const float HEIGHT_TOLERANCE = 0.3f;
        const float ENTER_XZ = CABIN_HALF_WIDTH * 0.8f;
        const float EXIT_XZ = CABIN_HALF_WIDTH * 1.1f;

        const float ELEVATOR_WALL_LIMIT = CABIN_HALF_WIDTH * 0.9f;
        const float ELEVATOR_COLLISION_ZONE = CABIN_HALF_WIDTH * 1.2f;

        if (!inLiftState) {
            float relX = player.position.x - elevatorFloorPos.x;
            float relZ = player.position.z - elevatorFloorPos.z;

            if (dy < HEIGHT_TOLERANCE) {
                if (relX < -CABIN_HALF_WIDTH && relX > -ELEVATOR_COLLISION_ZONE) {
                    if (std::fabs(relZ) < CABIN_HALF_DEPTH) {
                        player.position.x = elevatorFloorPos.x - ELEVATOR_COLLISION_ZONE;
                    }
                }
                if (relX > CABIN_HALF_WIDTH && relX < ELEVATOR_COLLISION_ZONE) {
                    if (std::fabs(relZ) < CABIN_HALF_DEPTH) {
                        player.position.x = elevatorFloorPos.x + ELEVATOR_COLLISION_ZONE;
                    }
                }
                if (relZ < -CABIN_HALF_DEPTH && relZ > -ELEVATOR_COLLISION_ZONE) {
                    if (std::fabs(relX) < CABIN_HALF_WIDTH) {
                        player.position.z = elevatorFloorPos.z - ELEVATOR_COLLISION_ZONE;
                    }
                }
            }
        }

        if (inLiftState && !elevatorSys.doorsOpen) {
            float relX = player.position.x - elevatorFloorPos.x;
            float relZ = player.position.z - elevatorFloorPos.z;

            if (relX < -ELEVATOR_WALL_LIMIT) player.position.x = elevatorFloorPos.x - ELEVATOR_WALL_LIMIT;
            if (relX > ELEVATOR_WALL_LIMIT) player.position.x = elevatorFloorPos.x + ELEVATOR_WALL_LIMIT;
            if (relZ < -ELEVATOR_WALL_LIMIT) player.position.z = elevatorFloorPos.z - ELEVATOR_WALL_LIMIT;
            if (relZ > ELEVATOR_WALL_LIMIT) player.position.z = elevatorFloorPos.z + ELEVATOR_WALL_LIMIT;
        }
        else if (inLiftState && elevatorSys.doorsOpen) {
            float relX = player.position.x - elevatorFloorPos.x;
            float relZ = player.position.z - elevatorFloorPos.z;

            if (relX < -ELEVATOR_WALL_LIMIT) player.position.x = elevatorFloorPos.x - ELEVATOR_WALL_LIMIT;
            if (relX > ELEVATOR_WALL_LIMIT) player.position.x = elevatorFloorPos.x + ELEVATOR_WALL_LIMIT;

            if (relZ < -ELEVATOR_WALL_LIMIT) player.position.z = elevatorFloorPos.z - ELEVATOR_WALL_LIMIT;
        }

        if (!inLiftState) {
            inLiftState = (dx < ENTER_XZ && dz < ENTER_XZ && dy < HEIGHT_TOLERANCE);
            if (inLiftState) {
                std::cout << "Entered elevator at floor " << elevatorSys.currentFloor << std::endl;
                needsRender = true;
            }
        }
        else {
            bool farEnoughAway = (dx >= EXIT_XZ || dz >= EXIT_XZ);
            if (farEnoughAway) {
                inLiftState = false;
                std::cout << "Exited elevator" << std::endl;
                needsRender = true;
            }
        }

        bool inLift = inLiftState;
        g_playerInElevator = inLift;

        if (CheckIfRenderNeeded(player, camera, elevatorSys)) {
            needsRender = true;
        }

        if (inLift) {
            bool rightMousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightMousePressed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                lookAroundMode = true;
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                if (lookAroundMode) {
                    firstMouse = true;
                    lookAroundMode = false;
                }
            }
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            lookAroundMode = false;
        }

        if (inLift) player.position.y = elevatorSys.y + 0.1f;

        bool cCurrent = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
        if (!inLift && cCurrent && !keyCPressed) {
            AddTarget(elevatorSys, playerFloor);
            std::cout << "Elevator called to floor " << playerFloor << std::endl;
            needsRender = true;
        }
        keyCPressed = cCurrent;

        std::vector<Button2D> buttons;

        if (inLift) {
            float panelX = SCR_W - 180.0f;
            float panelY = SCR_H / 2.0f;
            float btnW = 70.0f, btnH = 70.0f, spacing = 80.0f;

            buttons.push_back({ panelX - spacing, panelY + 170, btnW, btnH, tex_PR, 0, Button2D::Floor, elevatorSys.buttonLit[1] });
            buttons.push_back({ panelX + spacing, panelY + 170, btnW, btnH, tex_SU, -1, Button2D::Floor, elevatorSys.buttonLit[0] });
            buttons.push_back({ panelX - spacing, panelY + 80, btnW, btnH, tex_1, 1, Button2D::Floor, elevatorSys.buttonLit[2] });
            buttons.push_back({ panelX + spacing, panelY + 80, btnW, btnH, tex_2, 2, Button2D::Floor, elevatorSys.buttonLit[3] });
            buttons.push_back({ panelX - spacing, panelY, btnW, btnH, tex_3, 3, Button2D::Floor, elevatorSys.buttonLit[4] });
            buttons.push_back({ panelX + spacing, panelY, btnW, btnH, tex_4, 4, Button2D::Floor, elevatorSys.buttonLit[5] });
            buttons.push_back({ panelX - spacing, panelY - 80, btnW, btnH, tex_5, 5, Button2D::Floor, elevatorSys.buttonLit[6] });
            buttons.push_back({ panelX + spacing, panelY - 80, btnW, btnH, tex_6, 6, Button2D::Floor, elevatorSys.buttonLit[7] });
            buttons.push_back({ panelX - spacing, panelY - 180, btnW, btnH, tex_stop, 0, Button2D::Stop, false });
            buttons.push_back({ panelX + spacing, panelY - 180, btnW, btnH, tex_vent, 0, Button2D::Vent, elevatorSys.ventilationOn });
            buttons.push_back({ panelX - spacing, panelY - 260, btnW, btnH, tex_open, 0, Button2D::Open, false });
            buttons.push_back({ panelX + spacing, panelY - 260, btnW, btnH, tex_close, 0, Button2D::Close, false });
        }

        if (needsRender) {
            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 P = glm::perspective(glm::radians(camera.Zoom),
                (float)SCR_W / SCR_H, 0.1f, 200.0f);
            glm::mat4 V = camera.GetViewMatrix();

            shader.use();
            shader.setMat4("uP", P);
            shader.setMat4("uV", V);
            shader.setVec3("uViewPos", camera.Position);

            shader.setVec3("uLight.pos", 0.0f, camera.Position.y + 8.0f, 0.0f);
            shader.setVec3("uLight.kA", 0.4f, 0.4f, 0.4f);
            shader.setVec3("uLight.kD", 1.0f, 1.0f, 1.0f);
            shader.setVec3("uLight.kS", 1.0f, 1.0f, 1.0f);

            SetMaterialColor(shader, glm::vec3(0.0f, 1.0f, 1.0f), 64.0f);
            for (auto& f : floors) f.Draw(shader);

            for (int i = -1; i <= 6; i++)
            {
                float y = FloorToWorldY(i);

                SetMaterialColor(shader, glm::vec3(1.0f, 0.5f, 0.0f), 32.0f);

                glm::mat4 M(1.0f);
                M = glm::translate(M, glm::vec3(0.0f, y + 1.5f, -6.0f));
                shader.setMat4("uM", M);
                glBindVertexArray(wallVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                M = glm::mat4(1.0f);
                M = glm::translate(M, glm::vec3(-6.0f, y + 1.5f, 0.0f));
                M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));
                shader.setMat4("uM", M);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                M = glm::mat4(1.0f);
                M = glm::translate(M, glm::vec3(6.0f, y + 1.5f, 0.0f));
                M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0, 1, 0));
                shader.setMat4("uM", M);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                M = glm::mat4(1.0f);
                M = glm::translate(M, glm::vec3(0.0f, y + 1.5f, 6.0f));
                M = glm::rotate(M, glm::radians(180.0f), glm::vec3(0, 1, 0));
                shader.setMat4("uM", M);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            for (int i = -1; i <= 6; i++)
            {
                float y = FloorToWorldY(i);

                if (i == 0) {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, glm::vec3(0.0f, y, 4.0f));
                    M = glm::scale(M, glm::vec3(0.01f));
                    shader.setMat4("uM", M);
                    plantHouse.Draw(shader);
                }

                if (i % 3 == 0) {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, glm::vec3(-4.0f, y+0.080f, 3.0f));
                    M = glm::scale(M, glm::vec3(0.30f));
                    shader.setMat4("uM", M);
                    plantAloe.Draw(shader);
                }

                if (i % 3 == 1) {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, glm::vec3(4.0f, y+0.080f, 3.0f));
                    M = glm::scale(M, glm::vec3(0.30f));
                    shader.setMat4("uM", M);
                    plantHouse.Draw(shader);
                }

                if (i % 3 == 2) {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, glm::vec3(-4.0f, y+0.080f, 3.0f));
                    M = glm::scale(M, glm::vec3(0.30f));
                    shader.setMat4("uM", M);
                    plantAloe.Draw(shader);
                }

                if (i % 3 == 0 && i != -1) {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, glm::vec3(4.0f, y+0.50f, 3.0f));
                    M = glm::scale(M, glm::vec3(0.30f));
                    shader.setMat4("uM", M);
                    plantPot.Draw(shader);
                }
            }

            for (int i = -1; i <= 6; i++)
            {
                float y = FloorToWorldY(i);

                glm::mat4 M(1.0f);
                M = glm::translate(M, glm::vec3(-5.5f, y + 2.0f, 0.0f));
                M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));
                M = glm::scale(M, glm::vec3(0.015f));
                shader.setMat4("uM", M);
                wallLight.Draw(shader);

                M = glm::mat4(1.0f);
                M = glm::translate(M, glm::vec3(5.5f, y + 2.0f, 0.0f));
                M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0, 1, 0));
                M = glm::scale(M, glm::vec3(0.015f));
                shader.setMat4("uM", M);
                wallLight.Draw(shader);
            }

            {
                glm::mat4 M(1.0f);
                M = glm::translate(M, glm::vec3(0.0f, elevatorSys.y + ELEVATOR_Y_OFFSET, 0.0f));
                M = glm::scale(M, glm::vec3(ELEVATOR_SCALE));
                shader.setMat4("uM", M);
                elevatorModel.Draw(shader);
            }

            {
                float doorY = elevatorSys.y + DESIRED_ELEVATOR_HEIGHT * 0.48f;

                glm::mat4 M_left(1.0f);
                M_left = glm::translate(M_left, glm::vec3(elevatorSys.doorLeftOffset - 0.45f, doorY, DOOR_Z_OFFSET));
                M_left = glm::scale(M_left, glm::vec3(DOOR_SCALE_WIDTH, DOOR_SCALE_HEIGHT, DOOR_SCALE_HEIGHT));
                shader.setMat4("uM", M_left);
                doorModel.Draw(shader);

                glm::mat4 M_right(1.0f);
                M_right = glm::translate(M_right, glm::vec3(elevatorSys.doorRightOffset + 0.45f, doorY, DOOR_Z_OFFSET));
                M_right = glm::scale(M_right, glm::vec3(-DOOR_SCALE_WIDTH, DOOR_SCALE_HEIGHT, DOOR_SCALE_HEIGHT));
                shader.setMat4("uM", M_right);
                doorModel.Draw(shader);
            }

            if (inLift && !buttons.empty())
            {
                glDisable(GL_DEPTH_TEST);

                buttonShader.use();
                glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_W, 0.0f, (float)SCR_H);
                buttonShader.setMat4("uP", ortho);
                buttonShader.setMat4("uV", glm::mat4(1.0f));

                for (const auto& btn : buttons)
                    DrawButton(buttonShader, btn.texture, btn.cx, btn.cy, btn.w, btn.h, btn.active);

                if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
            }

            {
                glDisable(GL_DEPTH_TEST);

                buttonShader.use();
                glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_W, 0.0f, (float)SCR_H);
                buttonShader.setMat4("uP", ortho);
                buttonShader.setMat4("uV", glm::mat4(1.0f));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex_signature);
                buttonShader.setInt("uTex", 0);
                buttonShader.setInt("uActive", 0);

                glm::mat4 M = glm::mat4(1.0f);
                M = glm::translate(M, glm::vec3(SCR_W - 200.0f, 100.0f, 0.0f));
                M = glm::scale(M, glm::vec3(350.0f, 150.0f, 1.0f));
                buttonShader.setMat4("uM", M);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
            }

            glfwSwapBuffers(window);

            needsRender = false;
        }

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = SCR_H - my;
        bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (inLift && mouseDown && !mouseWasDown)
        {
            for (const auto& btn : buttons)
            {
                if (MouseInRect(mx, my, btn.cx, btn.cy, btn.w, btn.h))
                {
                    switch (btn.type)
                    {
                    case Button2D::Floor:
                        AddTarget(elevatorSys, btn.targetFloor);
                        needsRender = true;
                        break;
                    case Button2D::Open:
                        OpenDoors(elevatorSys, now);
                        needsRender = true;
                        break;
                    case Button2D::Close:
                        CloseDoors(elevatorSys);
                        needsRender = true;
                        break;
                    case Button2D::Stop:
                        StopElevator(elevatorSys);
                        needsRender = true;
                        break;
                    case Button2D::Vent:
                        elevatorSys.ventilationOn = !elevatorSys.ventilationOn;
                        if (elevatorSys.ventilationOn) elevatorSys.ventilationAutoOffOnNextStop = true;
                        needsRender = true;
                        break;
                    }
                    break;
                }
            }
        }
        mouseWasDown = mouseDown;

        glfwPollEvents();

        double frameEnd = glfwGetTime();
        double frameTime = frameEnd - frameStart;
        if (frameTime < FRAME_TIME) {
            double sleepTime = FRAME_TIME - frameTime;
            while (glfwGetTime() - frameEnd < sleepTime) {}
        }
    }

    glfwTerminate();
    return 0;
}