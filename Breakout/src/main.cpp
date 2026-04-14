#include "raylib.h"
#include "Game.h"
#include <cstdio>
#include "Ball.h"

void TestCollisionTrue() {
    Ball ball({100, 100}, {0, 0}, 10);

    ball.Launch(100, 100);   // ⭐ 加这一句！！！

    Rectangle brick = {90, 90, 50, 20};

    bool result = ball.CheckBrickCollision(brick);

    if (result)
        printf("Test 1 Passed\n");
    else
        printf("Test 1 Failed\n");
}

void TestCollisionFalse() {
    Ball ball({10, 10}, {0, 0}, 10);

    ball.Launch(10, 100);   // ⭐ 加！！！

    Rectangle brick = {200, 200, 50, 20};

    bool result = ball.CheckBrickCollision(brick);

    if (!result)
        printf("Test 2 Passed\n");
    else
        printf("Test 2 Failed\n");
}

int main() {
    InitWindow(800, 600, "Breakout");

    Game game;
    game.Init();

    TestCollisionTrue();
    TestCollisionFalse();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}