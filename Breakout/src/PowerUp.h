// PowerUp.h
#pragma once
#include "raylib.h"
#include <memory>
#include "PowerUpFactory.h"   // 提供 PowerUpType

class PowerUpEffect;
class Game;

class PowerUp {
public:
    Vector2 position;
    PowerUpType type;
    bool active;
    float speed;
    float radius;
    std::unique_ptr<PowerUpEffect> effect;

    PowerUp(Vector2 pos, PowerUpType t, std::unique_ptr<PowerUpEffect> eff);

    void Update(float dt);
    void Draw();
    bool CheckCollision(Rectangle paddleRect);
    void ApplyEffect(Game& game);
};