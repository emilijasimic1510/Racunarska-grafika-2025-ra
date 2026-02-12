#include "ElevatorSystem.h"
#include <cmath>

static constexpr float FLOOR_HEIGHT = 3.0f;
static constexpr float LIFT_SPEED = 3.0f; // Increased for smoother animation

static constexpr float DOOR_OPEN_TIME = 5.0f;
static constexpr float DOOR_SPEED = 2.0f; // Increased for faster door animation
static constexpr float DOOR_TRAVEL = 0.5f; // Increased for better visual separation

float FloorToWorldY(int logicalFloor)
{
    return logicalFloor * FLOOR_HEIGHT;
}

static bool IsInQueue(std::queue<int> q, int f)
{
    while (!q.empty())
    {
        if (q.front() == f) return true;
        q.pop();
    }
    return false;
}

void AddTarget(ElevatorSystem& e, int floorIndex)
{
    if (!IsValidFloor(floorIndex)) return;

    if (floorIndex == e.currentFloor && !e.doorsOpen) return;

    if (IsInQueue(e.queue, floorIndex)) return;

    e.queue.push(floorIndex);
    e.buttonLit[FloorToButtonIndex(floorIndex)] = true;
    e.stopPressed = false;
}

void StopElevator(ElevatorSystem& e)
{
    while (!e.queue.empty()) e.queue.pop();

    e.stopPressed = true;
    e.state = ElevatorState::Idle;

    // Reset door state
    e.doorsOpen = false;
    e.doorLeftOffset = 0.0f;
    e.doorRightOffset = 0.0f;
    e.doorExtendedOnce = false;

    e.buttonLit.fill(false);
}

void OpenDoors(ElevatorSystem& e, double timeNow)
{
    // Cannot open while moving
    if (e.state == ElevatorState::Moving) return;

    // If already open, extend time
    if (e.state == ElevatorState::DoorsOpen && e.doorsOpen)
    {
        if (!e.doorExtendedOnce)
        {
            e.doorOpenStart = timeNow;
            e.doorExtendedOnce = true;
        }
        return;
    }

    // Open doors
    e.state = ElevatorState::DoorsOpen;
    e.doorsOpen = true;
    e.doorOpenStart = timeNow;
    e.doorExtendedOnce = false;
}

void CloseDoors(ElevatorSystem& e)
{
    if (e.state == ElevatorState::DoorsOpen)
        e.state = ElevatorState::DoorsClosing;
}

void UpdateElevator(ElevatorSystem& e, float dt, double timeNow)
{
    // Cap delta time
    if (dt > 0.05f) dt = 0.05f;

    // If stopped, don't move
    if (e.stopPressed) return;

    switch (e.state)
    {
    case ElevatorState::Idle:
        if (!e.queue.empty() && !e.doorsOpen)
        {
            e.targetFloor = e.queue.front();
            e.queue.pop();
            e.state = ElevatorState::Moving;
        }
        break;

    case ElevatorState::Moving:
    {
        float targetY = FloorToWorldY(e.targetFloor);
        float dir = (targetY > e.y) ? 1.0f : -1.0f;
        float step = dir * LIFT_SPEED * dt;

        if ((dir > 0 && e.y + step >= targetY) ||
            (dir < 0 && e.y + step <= targetY))
        {
            e.y = targetY;
            e.currentFloor = e.targetFloor;

            // Light button
            if (IsValidFloor(e.currentFloor))
                e.buttonLit[FloorToButtonIndex(e.currentFloor)] = false;

            // Auto-off ventilation
            if (e.ventilationAutoOffOnNextStop)
            {
                e.ventilationOn = false;
                e.ventilationAutoOffOnNextStop = false;
            }

            // Open doors automatically
            e.state = ElevatorState::DoorsOpen;
            e.doorsOpen = true;
            e.doorOpenStart = timeNow;
            e.doorExtendedOnce = false;
        }
        else
        {
            e.y += step;
        }
        break;
    }

    case ElevatorState::DoorsOpen:
        e.doorLeftOffset -= DOOR_SPEED * dt;
        e.doorRightOffset += DOOR_SPEED * dt;

        if (e.doorLeftOffset < -DOOR_TRAVEL)
        {
            e.doorLeftOffset = -DOOR_TRAVEL;
            e.doorRightOffset = DOOR_TRAVEL;
        }

        if (timeNow - e.doorOpenStart >= DOOR_OPEN_TIME)
        {
            e.state = ElevatorState::DoorsClosing;
        }
        break;

    case ElevatorState::DoorsClosing:
        e.doorLeftOffset += DOOR_SPEED * dt;
        e.doorRightOffset -= DOOR_SPEED * dt;

        if (e.doorLeftOffset >= 0.0f)
        {
            e.doorLeftOffset = 0.0f;
            e.doorRightOffset = 0.0f;
            e.doorsOpen = false;

            if (!e.queue.empty())
            {
                e.targetFloor = e.queue.front();
                e.queue.pop();
                e.state = ElevatorState::Moving;
            }
            else
            {
                e.state = ElevatorState::Idle;
            }
        }
        break;
    }
}