#include "M13_Camera.h"
#include "raylib.h"

static Vector2 offset = {0, 0};

void InitCamera(Vector2 target) {
    offset = { GetScreenWidth()/2 - target.x, GetScreenHeight()/2 - target.y };
}

void UpdateCamera(Vector2 target) {
    // 简单的跟随：保持玩家在屏幕中心
    offset = { GetScreenWidth()/2 - target.x, GetScreenHeight()/2 - target.y };
}

Vector2 GetCameraOffset() {
    return offset;
}