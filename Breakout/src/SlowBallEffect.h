// SlowBallEffect.h
#pragma once
#include "PowerUpEffect.h"
#include <vector>

class Game;

class SlowBallEffect : public PowerUpEffect {
    float speedFactor;
    float duration;
    float timer;
    std::vector<float> originalSpeeds;
public:
    SlowBallEffect(float factor, float dur);
    void Apply(Game& game) override;
    void Update(Game& game, float dt) override;
    bool IsExpired() const override;
};