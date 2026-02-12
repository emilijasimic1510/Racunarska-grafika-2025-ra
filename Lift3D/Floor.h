#pragma once
#pragma once
#include <glm/glm.hpp>
#include "Shader.h"

class Floor {
public:
    glm::vec3 position;
    float size;

    Floor(glm::vec3 pos, float s);
    void Draw(Shader& shader);
};
