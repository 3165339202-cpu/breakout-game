#pragma once 
#include "raylib.h"
class Paddle {
public:
    Rectangle rect;
    float speed;

    void Init(float x, float y) {
        rect = {x, y, 100, 20};
        speed = 6;
    }

    void Update() {
        if (IsKeyDown(KEY_LEFT))
            rect.x -= speed;

        if (IsKeyDown(KEY_RIGHT))
            rect.x += speed;
    }

    void Draw() {
        DrawRectangleRec(rect, BLUE);
    }
};