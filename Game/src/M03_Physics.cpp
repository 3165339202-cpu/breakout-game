#include "M03_Physics.h"
#include "Common.h"
#include <vector>

float GRAVITY = 800.0f;

bool CheckCollision(Vector2 pos1, Vector2 size1, Vector2 pos2, Vector2 size2) {
    return (pos1.x < pos2.x + size2.x &&
            pos1.x + size1.x > pos2.x &&
            pos1.y < pos2.y + size2.y &&
            pos1.y + size1.y > pos2.y);
}

void MoveEntity(Entity& e, Vector2 delta, const std::vector<Entity*>& statics) {
    // 简单的移动，不考虑连续碰撞
    e.position.x += delta.x;
    e.position.y += delta.y;
    // 这里可以加入与 statics 的碰撞解析，基础版本先省略
}

void UpdatePhysics(float dt) {
    // 基础版本中，物理由角色状态机直接处理，这里留空
}