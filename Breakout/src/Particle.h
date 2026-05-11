#pragma once

#include "raylib.h"

struct Particle {
    Vector2 position{};
    Vector2 velocity{};
    Color color{WHITE};
    float life = 0.0f;      // 剩余寿命（秒）
    float maxLife = 1.0f;   // 用于保持原有淡出视觉效果
    float size = 2.0f;
    bool active = false;    // 对象池生命周期标记：inactive 时不删除，只等待复用

    Particle() = default;

    Particle(Vector2 pos, Vector2 vel, Color col, float lf = 1.0f, float sz = 2.0f) {
        Reset(pos, vel, col, lf, sz);
    }

    void Reset(Vector2 pos, Vector2 vel, Color col, float lf = 1.0f, float sz = 2.0f) {
        position = pos;
        velocity = vel;
        color = col;
        life = lf;
        maxLife = lf;
        size = sz;
        active = true;
    }

    void Update(float dt) {
        if (!active) return;

        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
        life -= dt;

        if (life <= 0.0f) {
            active = false;
            life = 0.0f;
        }
    }

    void Draw() const {
        if (!active) return;

        const float alpha = (maxLife > 0.0f) ? (life / maxLife) : 0.0f;
        DrawCircleV(position, size, Fade(color, alpha)); // 淡出效果
    }

    bool IsAlive() const { return active && life > 0.0f; }
};
