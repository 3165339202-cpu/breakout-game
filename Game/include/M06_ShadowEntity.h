#ifndef SHADOW_ENTITY_H
#define SHADOW_ENTITY_H

#include "Common.h"

struct Shadow : public Entity {
    float lifetime;          // 剩余存在时间（3秒）
    // 可存储历史轨迹，但为了简化，直接从影子记录器获取
};

void SpawnShadow(Vector2 pos);                      // 在指定位置生成影子
void UpdateShadows(float dt);                        // 更新所有影子的位置（从记录器读取）
void DrawShadows();                                  // 绘制所有影子

#endif
