#include "raylib.h"

int main()
{
    InitWindow(800,600,"test");

    Font font = LoadFontEx(
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        40,
        0,
        0
    );

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTextEx(font,"中文测试",(Vector2){100,100},40,2,BLACK);

        EndDrawing();
    }

    CloseWindow();
}