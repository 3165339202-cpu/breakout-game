// MultiBallEffect.h
#pragma once
#include "PowerUpEffect.h"

class Game;

class MultiBallEffect : public PowerUpEffect {
    int extraBalls;
public:
    MultiBallEffect(int n);
    void Apply(Game& game) override;
};