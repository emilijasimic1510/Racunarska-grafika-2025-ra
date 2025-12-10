#include "../Header/LiftApp.h"

//vao za pravougaonik
void setupRectVAO() {
    float vertices[] = {
        -0.5f,-0.5f, 1,1,1,
         0.5f,-0.5f, 1,1,1,
         0.5f, 0.5f, 1,1,1,
        -0.5f,-0.5f, 1,1,1,
         0.5f, 0.5f, 1,1,1,
        -0.5f, 0.5f, 1,1,1
    };

    unsigned int VBO;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// vao za teksturuv

void setupTextureVAO() {
    float vertices[] = {
        -0.5f,-0.5f, 0,0,
         0.5f,-0.5f, 1,0,
         0.5f, 0.5f, 1,1,
        -0.5f,-0.5f, 0,0,
         0.5f, 0.5f, 1,1,
        -0.5f, 0.5f, 0,1
    };

    unsigned int VBO;
    glGenVertexArrays(1, &VAOtex);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAOtex);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}



void drawRect(float cx, float cy, float sx, float sy, float r, float g, float b) {
    glUseProgram(rectShader);
    glBindVertexArray(VAOrect);

    glUniform2f(glGetUniformLocation(rectShader, "uPos"), cx, cy);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), sx, sy);
    glUniform3f(glGetUniformLocation(rectShader, "uColor"), r, g, b);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}



void drawButton(unsigned int tex, float cx, float cy, float w, float h) {
    glUseProgram(shaderTex);
    glBindVertexArray(VAOtex);

    glUniform2f(glGetUniformLocation(shaderTex, "uPos"), cx, cy);
    glUniform2f(glGetUniformLocation(shaderTex, "uScale"), w, h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(shaderTex, "uTex"), 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}


void drawButtonWithOutline(Button& b)
{
    drawButton(b.texture, b.cx, b.cy, b.sx, b.sy);

    if (!b.active) return;

    float t = 0.02f;

    drawRect(b.cx, b.cy + b.sy / 2 + t / 2, b.sx, t, 1, 1, 1);
    drawRect(b.cx, b.cy - b.sy / 2 - t / 2, b.sx, t, 1, 1, 1);
    drawRect(b.cx - b.sx / 2 - t / 2, b.cy, t, b.sy, 1, 1, 1);
    drawRect(b.cx + b.sx / 2 + t / 2, b.cy, t, b.sy, 1, 1, 1);
}



void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT);

    // panel
    drawRect(-0.5f, 0, 1, 2, 0.2f, 0.2f, 0.2f);
    // zgrada
    drawRect(0.5f, 0, 1, 2, 0.1f, 0.1f, 0.1f);

    // spratovi
    for (int i = 0; i < NUM_FLOORS; i++)
        drawRect(0.5f, gFloorY[i], 1, 0.02f, 0.3f, 0.3f, 0.3f);

    // kabina
    drawRect(gElevator.x, gElevator.y,
        gElevator.width, gElevator.height,
        0.8f, 0.8f, 0.1f);

    float doorW = gElevator.width * 0.25f;
    float doorH = gElevator.height * 0.9f;

    // levo krilo od vrata
    drawRect(gElevator.x - gElevator.width * 0.25f + gElevator.doorLeftOffset,
        gElevator.y, doorW, doorH, 0.9f, 0.9f, 0.9f);

    // desno krilo od vrata
    drawRect(gElevator.x + gElevator.width * 0.25f + gElevator.doorRightOffset,
        gElevator.y, doorW, doorH, 0.9f, 0.9f, 0.9f);

    // osoba da se vidi uvek van lifta a u liftu se ne vidi kada su zatvorena vrata
    if (!gPerson.inElevator)
    {
        drawButton(tex_person, gPerson.x, gPerson.y, gPerson.width, gPerson.height);
    }
    else
    {
        float eps = 0.0001f;
        bool doorsFullyOpen =
            (gElevator.doorLeftOffset <= -doorW + eps &&
                gElevator.doorRightOffset >= doorW - eps);

        if (doorsFullyOpen)
        {
            drawButton(tex_person, gPerson.x, gPerson.y, gPerson.width, gPerson.height);
        }
    }

    // dugmici
    drawButtonWithOutline(btnSU);
    drawButtonWithOutline(btnPR);

    for (int i = 0; i < 6; i++)
        drawButtonWithOutline(btnFloor[i]);

    drawButtonWithOutline(btnOpen);
    drawButtonWithOutline(btnClose);
    drawButtonWithOutline(btnStop);
    drawButtonWithOutline(btnVent);

    // potpis u gornjem levom uglu 
    drawButton(tex_signature, -0.85f, 0.85f, 0.25f, 0.10f);

}
