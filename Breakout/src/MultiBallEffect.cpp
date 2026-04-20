// MultiBallEffect.cpp
#include "MultiBallEffect.h"
#include "Game.h"
#include "Ball.h"
#include <cmath>

MultiBallEffect::MultiBallEffect(int n) : extraBalls(n) {}

void MultiBallEffect::Apply(Game& game) {
    for (int i = 0; i < extraBalls; ++i) {
        Ball newBall = game.GetBall(); // 拷贝主球
        float angle = (rand() % 360) * DEG2RAD;
        Vector2 vel = { cosf(angle) * 8.0f, sinf(angle) * 8.0f };
        newBall.SetSpeed(vel);
        newBall.SetLaunched(true); // 标记已发射，可能需要调整 Launch 参数
        game.AddBall(newBall);
    }
}