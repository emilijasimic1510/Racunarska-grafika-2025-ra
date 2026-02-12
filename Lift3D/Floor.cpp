#include "Floor.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

static unsigned int VAO = 0, VBO = 0;

Floor::Floor(glm::vec3 pos, float s)
{
    position = pos;
    size = s;

    if (VAO == 0) {
        float vertices[] = {
            -0.5f, 0.0f, -0.5f,
             0.5f, 0.0f, -0.5f,
             0.5f, 0.0f,  0.5f,
            -0.5f, 0.0f,  0.5f
        };

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
}

void Floor::Draw(Shader& shader)
{
    glm::mat4 M(1.0f);
    M = glm::translate(M, position);
    M = glm::scale(M, glm::vec3(size, 1.0f, size));

    shader.setMat4("uM", M);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}