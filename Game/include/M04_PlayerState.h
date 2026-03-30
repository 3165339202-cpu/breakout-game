#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

#include "Common.h"

struct Player : public Entity {
    bool onGround;
    bool canJump;
    float jumpCooldown;
    // 影子能量等
};

void InitPlayer(Player& player);
void UpdatePlayer(Player& player, Action action, float dt);

#endif
