#pragma once
#pragma once

enum class ButtonType {
    Floor,
    Open,
    Close,
    Stop,
    Vent
};

struct UIButton
{
    float x, y, w, h;
    ButtonType type;
    int floor;

    bool isHovered(double mx, double my, int sw, int sh) const
    {
        float px = x * sw;
        float py = y * sh;
        float pw = w * sw;
        float ph = h * sh;

        return mx >= px && mx <= px + pw &&
            my >= py && my <= py + ph;
    }
};
