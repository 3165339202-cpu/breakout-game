// ExtendPaddleEffect.h
#pragma once
#include "PowerUpEffect.h"

class Game; // 前向声明

class ExtendPaddleEffect : public PowerUpEffect {
    float extraWidth;
    float duration;
    float timer;
public:
    ExtendPaddleEffect(float w, float d);
    void Apply(Game& game) override;
    void Update(Game& game, float dt) override;
    bool IsExpired() const override;
};