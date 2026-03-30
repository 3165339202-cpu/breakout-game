#ifndef SHADOW_RECORDER_H
#define SHADOW_RECORDER_H

#include "Common.h"
#include "M04_PlayerState.h"

void InitRecorder(int maxFrames);                   // 初始化记录器，maxFrames对应3秒（60fps = 180帧）
void RecordFrame(const Player& player);             // 每帧调用，记录当前玩家状态
Vector2 GetPastPosition(float secondsAgo);          // 获取过去secondsAgo时的玩家位置（用于影子）
bool GetPastOnGround(float secondsAgo);             // 获取过去是否在地面（可选）

#endif