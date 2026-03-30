#pragma once
#include "raylib.h"

// 基类
class GameObject {
public:
    Vector2 position;

    GameObject(Vector2 pos = {0, 0}) {
        position = pos;
    }
};

// 物理类
class PhysicalObject : virtual public GameObject {
public:
    Vector2 velocity;
    float radius;

    PhysicalObject(Vector2 pos = {0,0}, Vector2 vel = {0,0}, float r = 0)
        : GameObject(pos) {
        velocity = vel;
        radius = r;
    }
};

// 渲染类
class VisualObject : virtual public GameObject {
public:
    Color color;
    bool visible;

    VisualObject(Vector2 pos = {0,0}, Color c = WHITE, bool v = true)
        : GameObject(pos) {
        color = c;
        visible = v;
    }
};