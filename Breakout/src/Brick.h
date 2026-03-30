#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

// ⭐ 新增：砖块类型
enum BrickType {
    NORMAL,
    EXPLOSIVE,
    GOLDEN
};

class Brick {
private:
    Rectangle rect;
    bool active;
    Color color;
    BrickType type;   // ⭐ 新增

public:
    // ⭐ 修改构造函数（多一个参数，默认普通砖）
    Brick(float x, float y, float width, float height, Color c, BrickType t = NORMAL);

    void Draw();

    bool IsActive() { return active; }
    void SetActive(bool a) { active = a; }
    Rectangle GetRect() { return rect; }

    BrickType GetType() { return type; }   // ⭐ 新增
};

#endif