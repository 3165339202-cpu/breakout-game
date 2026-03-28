#ifndef ANIMATION_H
#define ANIMATION_H

#include "Common.h"

void UpdateAnimations(float dt);         // 更新动画状态
void DrawAnimated(const Entity& e);      // 绘制带动画的实体（优先于基础绘制）

#endif