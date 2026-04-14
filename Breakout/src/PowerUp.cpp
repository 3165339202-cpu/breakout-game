// PowerUp.cpp
#include "PowerUp.h"
#include "Game.h"               // 为了使用 AddActiveEffect
#include "PowerUpEffect.h"      // 为了调用 Apply

PowerUp::PowerUp(Vector2 pos, PowerUpType t, std::unique_ptr<PowerUpEffect> eff)
    : position(pos), type(t), active(true), speed(150.0f), radius(15.0f), effect(std::move(eff))
{
}

void PowerUp::Update(float dt) {
    if (active) position.y += speed * dt;
}

void PowerUp::Draw() {
    if (!active) return;
    DrawCircleV(position, radius, YELLOW);
    DrawCircleLines(position.x, position.y, radius, ORANGE);
    const char* text = "";
    switch(type) {
        case PowerUpType::PADDLE_EXTEND: text = "P"; break;
        case PowerUpType::MULTI_BALL:    text = "M"; break;
        case PowerUpType::SLOW_BALL:     text = "S"; break;
    }
    DrawText(text, position.x-5, position.y-7, 20, BLACK);
}

bool PowerUp::CheckCollision(Rectangle paddleRect) {
    return CheckCollisionCircleRec(position, radius, paddleRect);
}

void PowerUp::ApplyEffect(Game& game) {
    if (effect) {
        effect->Apply(game);
        game.AddActiveEffect(std::move(effect));
    }
    active = false;
}