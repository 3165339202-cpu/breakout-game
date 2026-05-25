#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"
#include "GameObject.h"

/**
 * @brief 玩家挡板类，负责横向移动与临时增益效果。
 *
 * 是什么：
 * - 表示玩家可控挡板，维护宽高、屏幕边界、原始宽度与效果计时器。
 *
 * 为什么：
 * - 把输入位移和道具引发的尺寸变化封装在此类，保持游戏主循环简洁。
 *
 * 怎么用：
 * - 根据输入调用 `MoveLeft/MoveRight`；
 * - 每帧调用 `Update(dt)` 更新增益持续时间；
 * - 调用 `Extend(extraWidth, duration)` 启动“加长挡板”效果。
 */
class Paddle : public PhysicalObject, public VisualObject {
private:
    float width;
    float height;
    float screenWidth;
    float originalWidth;
    float effectTimer;

public:
    Paddle(float x, float y, float w, float h);

    void MoveLeft(float speed);
    void MoveRight(float speed);
    void Draw();
    void Extend(float extraWidth, float duration);
    void Update(float dt);

    Rectangle GetRect() const {
        return { position.x, position.y, width, height };
    }

    void SetX(float x) { position.x = x; }
    float GetX() const { return position.x; }
};

#endif
