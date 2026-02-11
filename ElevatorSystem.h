#pragma once
#include <queue>
#include <array>

enum class ElevatorState
{
    Idle,
    Moving,
    DoorsOpen,
    DoorsClosing
};

struct ElevatorSystem
{
   
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    int currentFloor = 0;
    int targetFloor = 0;

    ElevatorState state = ElevatorState::Idle;
    std::queue<int> queue;

    
    bool doorsOpen = false;
    float doorLeftOffset = 0.0f;
    float doorRightOffset = 0.0f;
    double doorOpenStart = 0.0;
    bool doorExtendedOnce = false;

   
    bool ventilationOn = false;
    bool ventilationAutoOffOnNextStop = false;

    
    bool stopPressed = false;

    
    std::array<bool, 8> buttonLit{ false,false,false,false,false,false,false,false };
};


inline bool IsValidFloor(int f) { return (f >= -1 && f <= 6); }
inline int  FloorToButtonIndex(int f) { return f + 1; }


float FloorToWorldY(int logicalFloor);

void AddTarget(ElevatorSystem& e, int floorIndex);
void UpdateElevator(ElevatorSystem& e, float dt, double timeNow);

void StopElevator(ElevatorSystem& e);
void OpenDoors(ElevatorSystem& e, double timeNow);
void CloseDoors(ElevatorSystem& e);
