#pragma once
#include "raylib.h"
class Brick {
public:
    Rectangle rect;
    bool active;

    void Init(float x, float y) {
        rect = {x, y, 60, 20};
        active = true;
    }

    void Draw() {
        if (active)
            DrawRectangleRec(rect, GREEN);
    }
};