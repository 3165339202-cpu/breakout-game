// ExtendPaddleEffect.cpp
#include "ExtendPaddleEffect.h"
#include "Game.h"

ExtendPaddleEffect::ExtendPaddleEffect(float w, float d) 
    : extraWidth(w), duration(d), timer(d) {}

void ExtendPaddleEffect::Apply(Game& game) {
    game.GetPaddle().Extend(extraWidth, duration);
}

void ExtendPaddleEffect::Update(Game& game, float dt) {
    timer -= dt;
}

bool ExtendPaddleEffect::IsExpired() const {
    return timer <= 0;
}