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
#include "ElevatorPanel.h"
#include "PlayerUtils.h"


int SCR_W = 1280, SCR_H = 720;

Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = 0.0f, lastY = 0.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static constexpr float LIFT_MODEL_Y_OFFSET = 2.0f;

Player player;


unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

void InitQuad()
{
    if (quadVAO) return;

    float vertices[] = {
        -0.5f,-0.5f,0.0f,
         0.5f,-0.5f,0.0f,
         0.5f, 0.5f,0.0f,

         0.5f, 0.5f,0.0f,
        -0.5f, 0.5f,0.0f,
        -0.5f,-0.5f,0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}


unsigned int LoadTexture(const wchar_t* filename)
{
    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;

    CoInitialize(nullptr);

    CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );

    factory->CreateDecoderFromFilename(
        filename,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

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
    CoUninitialize();

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
}


static bool MouseInRect(double mx, double my, float cx, float cy, float w, float h)
{
    return (mx > cx - w * 0.5 && mx < cx + w * 0.5 &&
        my > cy - h * 0.5 && my < cy + h * 0.5);
}

struct Btn2D
{
    float cx, cy, w, h;
    int targetFloor;
    enum Type { Floor, Open, Close, Stop, Vent } type;
};


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

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "3D Lift", monitor, nullptr);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();

    glViewport(0, 0, SCR_W, SCR_H);
    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader shader("res/shadders/phong.vert", "res/shadders/phong.frag");
    Shader panelShader("res/shadders/panel.vert", "res/shadders/panel.frag");

    InitQuad();

    unsigned int panelTexture =
        LoadTexture(L"res\\textures\\elevator_panel.png");

    Model elevator("res/elevator/elevator.obj");
    Model plantA("res/models/plants/AloePlant.obj");
    
    

    std::vector<Floor> floors;
    for (int i = -1; i <= 6; i++)
        floors.emplace_back(glm::vec3(0.0f, FloorToWorldY(i), 0.0f), 6.0f);

    ElevatorSystem elevatorSys;
    elevatorSys.currentFloor = 1;
    elevatorSys.y = FloorToWorldY(1);
    player.position.y = FloorToWorldY(1);
    player.position.x = -3.0f;   
    player.position.z = 0.0f;


    bool mouseWasDown = false;
    bool inLiftState = false;

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) player.position += forward * player.speed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) player.position -= forward * player.speed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) player.position -= right * player.speed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) player.position += right * player.speed * deltaTime;

        camera.Position = player.position + glm::vec3(0.0f, 1.6f, 3.0f);

        UpdateElevator(elevatorSys, deltaTime, now);

        glm::vec3 liftCenter(0.0f, elevatorSys.y, 0.0f);
        float dx = player.position.x - liftCenter.x;
        float dz = player.position.z - liftCenter.z;
        float dy = std::fabs(player.position.y - liftCenter.y);

        const float ENTER_XZ = 1.25f;
        const float ENTER_Y = 0.75f;
        const float EXIT_XZ = 1.55f;
        const float EXIT_Y = 1.05f;

        if (!inLiftState)
            inLiftState = (std::fabs(dx) < ENTER_XZ && std::fabs(dz) < ENTER_XZ && dy < ENTER_Y);
        else
            inLiftState = (std::fabs(dx) < EXIT_XZ && std::fabs(dz) < EXIT_XZ && dy < EXIT_Y);

        bool inLift = inLiftState;

        glfwSetInputMode(window, GLFW_CURSOR,
            inLift ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        if (inLift)
            player.position.y = elevatorSys.y;

        float panelX = SCR_W - 260.0f;
        float panelY = SCR_H / 2.0f;
        float panelW = 240.0f;
        float panelH = 520.0f;

        std::vector<Btn2D> btns;

      
        btns.push_back({ panelX - 60, panelY + 170, 80, 40, 0, Btn2D::Floor });
        btns.push_back({ panelX + 60, panelY + 170, 80, 40, -1, Btn2D::Floor });

   
        btns.push_back({ panelX - 60, panelY + 100, 80, 40, 1, Btn2D::Floor });
        btns.push_back({ panelX + 60, panelY + 100, 80, 40, 2, Btn2D::Floor });
        btns.push_back({ panelX - 60, panelY + 40, 80, 40, 3, Btn2D::Floor });
        btns.push_back({ panelX + 60, panelY + 40, 80, 40, 4, Btn2D::Floor });
        btns.push_back({ panelX - 60, panelY - 20, 80, 40, 5, Btn2D::Floor });
        btns.push_back({ panelX + 60, panelY - 20, 80, 40, 6, Btn2D::Floor });

        btns.push_back({ panelX - 60, panelY - 110, 90, 90, 0, Btn2D::Stop });
        btns.push_back({ panelX + 60, panelY - 110, 90, 90, 0, Btn2D::Vent });

        btns.push_back({ panelX - 60, panelY - 200, 100, 40, 0, Btn2D::Open });
        btns.push_back({ panelX + 60, panelY - 200, 100, 40, 0, Btn2D::Close });

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        my = SCR_H - my;

        bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (inLift && mouseDown && !mouseWasDown)
        {
            for (const auto& b : btns)
            {
                if (MouseInRect(mx, my, b.cx, b.cy, b.w, b.h))
                {
                    switch (b.type)
                    {
                    case Btn2D::Floor:
                        AddTarget(elevatorSys, b.targetFloor);
                        break;

                    case Btn2D::Open:
                        OpenDoors(elevatorSys, now);
                        break;

                    case Btn2D::Close:
                        CloseDoors(elevatorSys);
                        break;

                    case Btn2D::Stop:
                        StopElevator(elevatorSys);
                        break;

                    case Btn2D::Vent:
                        elevatorSys.ventilationOn = !elevatorSys.ventilationOn;
                        if (elevatorSys.ventilationOn)
                            elevatorSys.ventilationAutoOffOnNextStop = true;
                        break;
                    }
                    break;
                }
            }
        }

        mouseWasDown = mouseDown;

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 P = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_W / SCR_H, 0.01f, 200.0f);
        glm::mat4 V = camera.GetViewMatrix();

        shader.use();
        shader.setMat4("uP", P);
        shader.setMat4("uV", V);
        shader.setVec3("uViewPos", camera.Position);
        shader.setVec3("uLight.pos", 0.0f, elevatorSys.y + 6.0f, 4.0f);
        shader.setVec3("uLight.kA", 0.4f, 0.4f, 0.4f);
        shader.setVec3("uLight.kD", 0.9f, 0.9f, 0.9f);
        shader.setVec3("uLight.kS", 1.0f, 1.0f, 1.0f);

        for (auto& f : floors) f.Draw(shader);

        for (int i = -1; i <= 6; i++)
        {
            float y = FloorToWorldY(i);

            glm::mat4 PM(1.0f);
            PM = glm::translate(PM, glm::vec3(-2.5f, y + 0.35f, -2.0f));
            PM = glm::scale(PM, glm::vec3(0.25f));

            shader.setMat4("uM", PM);

            if (i % 3 == 0)
                plantA.Draw(shader);
            
           
        }

        for (int i = -1; i <= 6; i++)
        {
            float y = FloorToWorldY(i);

            glm::mat4 PM(1.0f);
            PM = glm::translate(PM, glm::vec3(-2.5f, y + 0.05f, -2.0f));
            PM = glm::scale(PM, glm::vec3(0.3f));

            
        }

        glm::mat4 M(1.0f);
        M = glm::translate(M, glm::vec3(0.0f, elevatorSys.y + LIFT_MODEL_Y_OFFSET, 0.0f));
        M = glm::scale(M, glm::vec3(0.01f));
        shader.setMat4("uM", M);
        elevator.Draw(shader);

        if (inLift)
        {
            glDisable(GL_DEPTH_TEST);
            panelShader.use();

            glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_W, 0.0f, (float)SCR_H);
            panelShader.setMat4("uP", ortho);
            panelShader.setMat4("uV", glm::mat4(1.0f));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, panelTexture);
            panelShader.setInt("uTex", 0);

            glBindVertexArray(quadVAO);

            glm::mat4 HM = glm::translate(glm::mat4(1.0f), glm::vec3(panelX, panelY, 0.0f));
            HM = glm::scale(HM, glm::vec3(panelW, panelH, 1.0f));

            panelShader.setMat4("uM", HM);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);




        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
