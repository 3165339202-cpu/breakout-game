#include "raylib.h"
#pragma once

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;      // 剩余寿命（秒）
    float size;
    
    Particle(Vector2 pos, Vector2 vel, Color col, float lf = 1.0f, float sz = 2.0f)
        : position(pos), velocity(vel), color(col), life(lf), size(sz) {}
    
    void Update(float dt) {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
        life -= dt;
    }
    
    void Draw() {
        DrawCircleV(position, size, Fade(color, life)); // 淡出效果
    }
    
    bool IsAlive() const { return life > 0; }
};