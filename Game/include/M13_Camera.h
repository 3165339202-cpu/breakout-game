#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"

void InitCamera(Vector2 target);         // 初始化摄像机
void UpdateCamera(Vector2 target);       // 每帧更新摄像机位置
Vector2 GetCameraOffset();               // 获取摄像机偏移（用于绘制时转换坐标）

#endif
