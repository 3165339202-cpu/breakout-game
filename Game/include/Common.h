#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <vector>
#include <memory>

// 实体类型枚举
enum class EntityType {
    PLAYER,
    SHADOW,
    PLATFORM,
    PRESSURE_PLATE,
    DOOR,
    ENEMY,
    MOVING_PLATFORM,
    // ... 其他
};

// 基础实体结构（所有游戏对象的基础）
struct Entity {
    EntityType type;
    Vector2 position;
    Vector2 size;       // 宽高，用于碰撞
    bool active;
    // 可根据需要扩展
};

// 全局实体列表（由核心引擎维护）
extern std::vector<std::unique_ptr<Entity>> g_entities;

// 动作枚举（输入）
enum class Action {
    NONE,
    LEFT,
    RIGHT,
    JUMP,
    SHADOW,
    PAUSE
};

#endif
