#pragma once
#include <vector>

enum class PanelAction {
    SU,
    PR,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    Open,
    Close,
    Stop,
    Vent
};

struct PanelButton2D {
    float x, y;
    float w, h;
    PanelAction action;
};

struct ElevatorPanel {

    std::vector<PanelButton2D> buttons;

    void Build(float panelX, float panelY)
    {
        buttons.clear();

        float startY = panelY + 160.0f;
        float step = 50.0f;

        auto add = [&](float offsetY, PanelAction a)
            {
                buttons.push_back({
                    panelX,
                    startY - offsetY,
                    70.0f,
                    35.0f,
                    a
                    });
            };

        add(0 * step, PanelAction::SU);
        add(1 * step, PanelAction::PR);
        add(2 * step, PanelAction::F1);
        add(3 * step, PanelAction::F2);
        add(4 * step, PanelAction::F3);
        add(5 * step, PanelAction::F4);
        add(6 * step, PanelAction::F5);
        add(7 * step, PanelAction::F6);

        add(9 * step, PanelAction::Open);
        add(10 * step, PanelAction::Close);
        add(11 * step, PanelAction::Stop);
        add(12 * step, PanelAction::Vent);
    }
};
