#ifndef BALL_H
#define BALL_H

#include "raylib.h"
#include "GameObject.h"

/**
 * @brief 小球实体，负责移动、重力、碰撞与发射控制。
 *
 * 是什么：
 * - 继承物理对象与可视对象，维护位置、速度、半径以及发射状态。
 *
 * 为什么：
 * - 将“与球相关的规则”封装在单一类中，便于多球模式复用同一逻辑。
 *
 * 怎么用：
 * - 每帧调用 `Move()/ApplyGravity()/Bounce*()` 更新物理；
 * - 调用 `Draw()` 进行渲染；
 * - 通过 `Launch/ResetToPaddle/Reset` 控制球的生命周期。
 */
class Ball : public PhysicalObject, public VisualObject {
private:
    float gravity;
    float maxSpeed;
    float bounceForce;
    bool launched;
    float launchCooldown;

public:
    Ball(Vector2 pos, Vector2 sp, float r);

    void Move();
    void Draw();
    void ApplyGravity();
    void BounceEdge(int screenWidth, int screenHeight);
    void BouncePaddle(Rectangle paddleRect);
    bool CheckBrickCollision(Rectangle brickRect);
    void SetLaunched(bool l) { launched = l; }

    void Launch(float paddleX, float paddleWidth);
    void ResetToPaddle(float paddleX, float paddleY);
    void Reset(Vector2 pos, Vector2 sp);
    void AddBounceForce(float force);

    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    Vector2 GetSpeed() const { return velocity; }

    void SetSpeed(Vector2 sp) { velocity = sp; }
    bool IsLaunched() const { return launched; }
};

#endif
