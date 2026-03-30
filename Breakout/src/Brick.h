#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"
#include "GameObject.h"   // ⭐ 新加

// ⭐ 砖块类型
enum BrickType {
    NORMAL,
    EXPLOSIVE,
    GOLDEN
};

class Brick : public VisualObject {   // ⭐ 继承！
private:
    Rectangle rect;
    bool active;
    BrickType type;

public:
    // ⭐ 构造函数
    Brick(float x, float y, float width, float height, Color c, BrickType t = NORMAL);

    void Draw();

    bool IsActive() { return active; }
    void SetActive(bool a) { active = a; }
    Rectangle GetRect() { return rect; }

    BrickType GetType() { return type; }
};

#endif