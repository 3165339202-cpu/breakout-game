#include "raylib.h"
#include "Game.h"

int main() {
    InitWindow(800, 600, "Breakout");

    Game game;
    game.Init();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}