#pragma once
#include "raylib.h"
class Game;
class PowerUpEffect {
public:
    virtual void Apply(Game& game)=0;
    virtual void Update(Game& game,float dt){}
    virtual bool IsExpired() const { return true; }
    virtual ~PowerUpEffect() = default;
};