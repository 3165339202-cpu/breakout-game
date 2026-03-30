#ifndef PHYSICS_H
#define PHYSICS_H

#include "Common.h"

// 重力常数（外部可配置）
extern float GRAVITY;

// 更新所有实体的物理（重力、移动），并处理与静态实体的碰撞
void UpdatePhysics(float dt);

// 检查两个AABB是否相交
bool CheckCollision(Vector2 pos1, Vector2 size1, Vector2 pos2, Vector2 size2);

// 移动实体并处理碰撞（由角色状态机调用）
void MoveEntity(Entity& e, Vector2 delta, const std::vector<Entity*>& statics);

#endif