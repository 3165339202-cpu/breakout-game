// SlowBallEffect.cpp
#include "SlowBallEffect.h"
#include "Game.h"
#include "Ball.h"
#include <cmath>

SlowBallEffect::SlowBallEffect(float factor, float dur)
    : speedFactor(factor), duration(dur), timer(dur) {}

void SlowBallEffect::Apply(Game& game) {
    originalSpeeds.clear();
    for (auto& ball : game.GetBalls()) {
        Vector2 vel = ball.GetSpeed();
        float originalSpeed = sqrt(vel.x*vel.x + vel.y*vel.y);
        originalSpeeds.push_back(originalSpeed);
        vel.x *= speedFactor;
        vel.y *= speedFactor;
        ball.SetSpeed(vel);
    }
}

void SlowBallEffect::Update(Game& game, float dt) {
    timer -= dt;
    if (timer <= 0) {
        int idx = 0;
        for (auto& ball : game.GetBalls()) {
            Vector2 vel = ball.GetSpeed();
            if (idx < (int)originalSpeeds.size()) {
                float currentSpeed = sqrt(vel.x*vel.x + vel.y*vel.y);
                if (currentSpeed > 0) {
                    float factor = originalSpeeds[idx] / currentSpeed;
                    vel.x *= factor;
                    vel.y *= factor;
                    ball.SetSpeed(vel);
                }
            }
            idx++;
        }
    }
}

bool SlowBallEffect::IsExpired() const {
    return timer <= 0;
}