
#include "raylib.h"

int main() {
    InitWindow(800, 600, "My First Game");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello Game!", 320, 280, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
}
