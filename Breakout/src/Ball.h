#ifndef BALL_H
#define BALL_H

#include "raylib.h"
#include "GameObject.h"

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