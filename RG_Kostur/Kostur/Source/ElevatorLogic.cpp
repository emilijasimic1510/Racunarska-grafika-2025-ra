#include "../Header/LiftApp.h"
#include <cmath>



// mapa spratova
float floorIndexToY(int logicalFloor) {
    int idx = logicalFloor + 1;      
    if (idx < 0) idx = 0;
    if (idx > 7) idx = 7;

    float bottom = -0.9f;
    float top = 0.9f;
    float step = (top - bottom) / float(NUM_FLOORS - 1);

    return bottom + idx * step;
}

// inicijalizacija dugmica
void initButtons() {
    float centerX = -0.5f;
    float row1 = 0.25f;
    float row2 = 0.00f;
    float row3 = -0.25f;
    float spacing = 0.22f;

    btnSU = { centerX - 1.5f * spacing, row1, 0.18f, 0.18f, tex_SU };
    btnPR = { centerX - 0.5f * spacing, row1, 0.18f, 0.18f, tex_PR };
    btnFloor[0] = { centerX + 0.5f * spacing, row1, 0.18f, 0.18f, tex_1 };
    btnFloor[1] = { centerX + 1.5f * spacing, row1, 0.18f, 0.18f, tex_2 };

    btnFloor[2] = { centerX - 1.5f * spacing, row2, 0.18f, 0.18f, tex_3 };
    btnFloor[3] = { centerX - 0.5f * spacing, row2, 0.18f, 0.18f, tex_4 };
    btnFloor[4] = { centerX + 0.5f * spacing, row2, 0.18f, 0.18f, tex_5 };
    btnFloor[5] = { centerX + 1.5f * spacing, row2, 0.18f, 0.18f, tex_6 };

    btnOpen = { centerX - 1.5f * spacing, row3, 0.18f, 0.18f, tex_open };
    btnClose = { centerX - 0.5f * spacing, row3, 0.18f, 0.18f, tex_close };
    btnStop = { centerX + 0.5f * spacing, row3, 0.18f, 0.18f, tex_stop };
    btnVent = { centerX + 1.5f * spacing, row3, 0.18f, 0.18f, tex_vent };
}

static GLFWcursor* createPropellerCursor()
{
    const int W = 32, H = 32;
    static unsigned char pixels[W * H * 4];

    int cx = W / 2;
    int cy = H / 2;

    for (int i = 0; i < W * H * 4; i++)
        pixels[i] = 0;

    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            int idx = (y * W + x) * 4;

            int dx = x - cx;
            int dy = y - cy;

            unsigned char r = 0, g = 0, b = 0, a = 0;

            if (std::abs(dy) < 3 && x > cx) { r = 255; g = 0;   b = 0;   a = 255; }
            else if (std::abs(dy) < 3 && x < cx) { r = 0;   g = 255; b = 0;   a = 255; }
            else if (std::abs(dx) < 3 && y > cy) { r = 0;   g = 0;   b = 255; a = 255; }
            else if (std::abs(dx) < 3 && y < cy) { r = 255; g = 255; b = 0;   a = 255; }

            if (std::abs(dx) <= 2 && std::abs(dy) <= 2)
            {
                r = 255; g = 255; b = 255; a = 255;
            }

            pixels[idx + 0] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = a;
        }

    GLFWimage img;
    img.width = W;
    img.height = H;
    img.pixels = pixels;

    return glfwCreateCursor(&img, cx, cy);
}



void addElevatorTarget(int floorIndex)
{
    gElevator.queue.push(floorIndex);
}



void updateElevator(float dt, double timeNow)
{
    // 1) Vrata otvorena → ceka se 5 sekundi
    if (gElevator.state == ElevatorState::DoorsOpen)
    {
        if (timeNow - gElevator.doorOpenStart >= DOOR_OPEN_TIME)
        {
            gElevator.doorsOpen = false;
            gElevator.state = ElevatorState::Idle;
            gElevator.doorOpening = false;
            gElevator.doorClosing = true;
        }
    }

    // 2) iz idle stanja do sledeceg  sprata 
    if (gElevator.state == ElevatorState::Idle && !gElevator.queue.empty())
    {
        gElevator.targetFloor = gElevator.queue.front();
        gElevator.queue.pop();
        gElevator.state = ElevatorState::Moving;
    }

    // 3) Kretanje lifta
    if (gElevator.state == ElevatorState::Moving)
    {
        float targetY = floorIndexToY(gElevator.targetFloor);
        float dir = (targetY > gElevator.y) ? 1.0f : -1.0f;

        gElevator.y += dir * LIFT_SPEED * dt;

        if ((dir > 0 && gElevator.y >= targetY) ||
            (dir < 0 && gElevator.y <= targetY))
        {
            gElevator.y = targetY;
            gElevator.currentFloor = gElevator.targetFloor;

            gElevator.doorsOpen = true;
            gElevator.state = ElevatorState::DoorsOpen;
            gElevator.doorOpenStart = timeNow;

            gElevator.doorOpening = true;
            gElevator.doorClosing = false;

            gElevator.openExtended = false;

            if (gElevator.currentFloor == -1) btnSU.active = false;
            else if (gElevator.currentFloor == 0) btnPR.active = false;
            else if (gElevator.currentFloor >= 1 && gElevator.currentFloor <= 6)
                btnFloor[gElevator.currentFloor - 1].active = false;
        }
    }

    // 4) Animacija vrata
    float travel = gElevator.width * 0.25f;

    if (gElevator.doorOpening)
    {
        gElevator.doorLeftOffset -= DOOR_ANIM_SPEED * dt;
        gElevator.doorRightOffset += DOOR_ANIM_SPEED * dt;

        if (gElevator.doorLeftOffset <= -travel)
        {
            gElevator.doorLeftOffset = -travel;
            gElevator.doorRightOffset = travel;
            gElevator.doorOpening = false;
        }
    }

    if (gElevator.doorClosing)
    {
        gElevator.doorLeftOffset += DOOR_ANIM_SPEED * dt;
        gElevator.doorRightOffset -= DOOR_ANIM_SPEED * dt;

        if (gElevator.doorLeftOffset >= 0.0f)
        {
            gElevator.doorLeftOffset = 0.0f;
            gElevator.doorRightOffset = 0.0f;
            gElevator.doorClosing = false;
            gElevator.openExtended = false;
        }
    }
}



void updatePerson(float dt)
{
    if (!gPerson.inElevator)
    {
        gPerson.y = floorIndexToY(gPerson.floor) - 0.5f * gPerson.height;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            gPerson.x -= PERSON_SPEED * 0.02f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            gPerson.x += PERSON_SPEED * 0.02f;

        if (gPerson.x < 0.1f)
            gPerson.x = 0.1f;

        float liftLeftEdge = gElevator.x - gElevator.width * 0.5f;
        if (gPerson.x + gPerson.width * 0.5f > liftLeftEdge)
            gPerson.x = liftLeftEdge - gPerson.width * 0.5f;
    }
    else
    {
        //centriranje 
        gPerson.x = gElevator.x;
        gPerson.y = gElevator.y - 0.5f * gPerson.height;
    }

    // ulazak
    if (!gPerson.inElevator && gElevator.doorsOpen)
    {
        float liftLeft = gElevator.x - gElevator.width * 0.5f;
        float personRight = gPerson.x + gPerson.width * 0.5f;

        if (gPerson.floor == gElevator.currentFloor &&
            personRight >= liftLeft - 0.01f)
        {
            gPerson.inElevator = true;
        }
    }

    // izlazak
    if (gPerson.inElevator && gElevator.doorsOpen)
    {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            gPerson.inElevator = false;
            gPerson.floor = gElevator.currentFloor;

            float liftLeft = gElevator.x - gElevator.width * 0.5f;
            float maxX = liftLeft - gPerson.width * 0.5f;
            if (gPerson.x > maxX)
                gPerson.x = maxX;
        }
    }
}



void keyCallback(GLFWwindow*, int key, int, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_C && !gPerson.inElevator)
    {
        float liftLeft = gElevator.x - gElevator.width * 0.5f;
        float personRight = gPerson.x + gPerson.width * 0.5f;

        if (personRight >= liftLeft - 0.01f)
            addElevatorTarget(gPerson.floor);
    }
}



void mouseButtonCallback(GLFWwindow* win, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    if (!gPerson.inElevator) return;

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);

    float nx = float(mx / winWidth) * 2.0f - 1.0f;
    float ny = 1.0f - float(my / winHeight) * 2.0f;

    auto hit = [&](Button& b)
        {
            return (nx > b.cx - b.sx / 2 && nx < b.cx + b.sx / 2 &&
                ny > b.cy - b.sy / 2 && ny < b.cy + b.sy / 2);
        };


    if (hit(btnSU)) { addElevatorTarget(-1); btnSU.active = true; }
    if (hit(btnPR)) { addElevatorTarget(0);  btnPR.active = true; }

    if (hit(btnFloor[0])) { addElevatorTarget(1); btnFloor[0].active = true; }
    if (hit(btnFloor[1])) { addElevatorTarget(2); btnFloor[1].active = true; }
    if (hit(btnFloor[2])) { addElevatorTarget(3); btnFloor[2].active = true; }
    if (hit(btnFloor[3])) { addElevatorTarget(4); btnFloor[3].active = true; }
    if (hit(btnFloor[4])) { addElevatorTarget(5); btnFloor[4].active = true; }
    if (hit(btnFloor[5])) { addElevatorTarget(6); btnFloor[5].active = true; }


    if (hit(btnOpen))
    {
        double now = glfwGetTime();

       
        if (!gElevator.doorsOpen && gElevator.state == ElevatorState::Idle)
        {
            gElevator.doorsOpen = true;
            gElevator.state = ElevatorState::DoorsOpen;
            gElevator.doorOpenStart = now;

            gElevator.doorOpening = true;
            gElevator.doorClosing = false;
            gElevator.openExtended = false;
        }
        
        else if (gElevator.doorsOpen && !gElevator.openExtended)
        {
            gElevator.doorOpenStart = now;
            gElevator.openExtended = true;
           
        }
    }


    // CLOSE – odmah zatvara
    if (hit(btnClose))
    {
        if (gElevator.doorsOpen)
        {
            gElevator.doorClosing = true;
            gElevator.doorOpening = false;
            gElevator.doorsOpen = false;
            gElevator.doorOpenStart = glfwGetTime() - DOOR_OPEN_TIME;
            gElevator.openExtended = false;
        }
    }

    // STOP
    if (hit(btnStop))
        gElevator.state = ElevatorState::Stopped;

    // ventil 
    if (hit(btnVent))
    {
        ventActive = !ventActive;

        if (ventActive)
        {
            if (!gPropellerCursor)
                gPropellerCursor = createPropellerCursor();

            glfwSetCursor(win, gPropellerCursor);
        }
        else
        {
            glfwSetCursor(win, NULL);
        }
    }
}
