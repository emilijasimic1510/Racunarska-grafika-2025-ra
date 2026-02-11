#pragma once
#pragma once
#include "Player.h"
#include "ElevatorSystem.h"
#include <glm/glm.hpp>

inline bool IsPlayerInElevator(const Player& p, const ElevatorSystem& e)
{
   
    glm::vec3 cabinPos(0.0f, e.y, 0.0f);

   
    glm::vec3 halfSize(1.0f, 1.5f, 1.0f);

    return
        p.position.x >= cabinPos.x - halfSize.x &&
        p.position.x <= cabinPos.x + halfSize.x &&
        p.position.z >= cabinPos.z - halfSize.z &&
        p.position.z <= cabinPos.z + halfSize.z &&
        p.position.y >= cabinPos.y &&
        p.position.y <= cabinPos.y + 3.0f;
}
