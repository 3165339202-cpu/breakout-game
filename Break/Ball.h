#pragma once
#include "raylib.h"

class Ball {
public:
    Vector2 position;
    Vector2 speed;
    float radius;

    void Init(float x, float y) {
        position = {x,y};
        speed = {4,-4};
        radius = 10;
    }

    void Update() {
        position.x += speed.x;
        position.y += speed.y;
    }

    void Draw() {
        DrawCircleV(position, radius, RED);
    }
};