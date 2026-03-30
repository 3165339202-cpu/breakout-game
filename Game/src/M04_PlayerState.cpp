#include "M04_PlayerState.h"
#include "M03_Physics.h"
#include <cstdio>  
#include "raylib.h"

void InitPlayer(Player& player) {
    player.type = EntityType::PLAYER;
    player.size = {30, 30};
    player.position = {100, 500};
    player.onGround = false;
    player.canJump = true;
}

void UpdatePlayer(Player& player, Action action, float dt) {
    // 简单的重力
    float vy = 0;
    if (!player.onGround) {
        vy = GRAVITY * dt;
    }

    // 水平移动
    float vx = 0;
    if (action == Action::LEFT) vx = -200 * dt;
    if (action == Action::RIGHT) vx = 200 * dt;

    // 跳跃
    if (action == Action::JUMP && player.onGround) {
        vy = -400;  // 瞬时速度
        player.onGround = false;
    }

    // 更新位置
    player.position.x += vx;
    player.position.y += vy;

    // 简单的平台碰撞检测（假设有一个静态平台在 y=550 处，宽 200，高 20）
    float groundY = 550;
    if (player.position.y + player.size.y >= groundY && 
        player.position.y + player.size.y - vy <= groundY + 5 &&
        player.position.x < 400 && player.position.x + player.size.x > 200) {
        player.position.y = groundY - player.size.y;
        player.onGround = true;
        vy = 0;
    } else {
        player.onGround = false;
    }

    if (action == Action::LEFT) printf("LEFT\n");
}





// #include "M04_PlayerState.h"
// #include "M03_Physics.h"
// #include <cstdio>  
// #include "raylib.h"

// void InitPlayer(Player& player) {
//     player.type = EntityType::PLAYER;
//     player.size = {30, 30};
//     player.position = {100, 500};
//     player.onGround = false;
//     player.canJump = true;
// }

// void UpdatePlayer(Player& player, Action action, float dt) {
//     // 简单的重力
//     float vy = 0;
//     if (!player.onGround) {
//         vy = GRAVITY * dt;
//     }

//     // 水平移动
//     float vx = 0;
//     if (action == Action::LEFT) vx = -200 * dt;
//     if (action == Action::RIGHT) vx = 200 * dt;

//     // 跳跃
//     if (action == Action::JUMP && player.onGround) {
//         vy = -400;  // 瞬时速度
//         player.onGround = false;
//     }

//     // 更新位置
//     player.position.x += vx;
//     player.position.y += vy;

//     // 简单的平台碰撞检测（假设有一个静态平台在 y=550 处，宽 200，高 20）
//     float groundY = 550;
//     if (player.position.y + player.size.y >= groundY && 
//         player.position.y + player.size.y - vy <= groundY + 5 &&
//         player.position.x < 400 && player.position.x + player.size.x > 200) {
//         player.position.y = groundY - player.size.y;
//         player.onGround = true;
//         vy = 0;
//     } else {
//         player.onGround = false;
//     }

//     if (action == Action::LEFT) printf("LEFT\n");
// }
