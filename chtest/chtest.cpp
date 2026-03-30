#include "raylib.h"

int main()
{
    InitWindow(800,600,"Chinese Test");

    const char *text = u8"中文测试";

    // 生成Unicode字符列表
    int codepoints[65536];
    for(int i=0;i<65536;i++)
    {
        codepoints[i] = i;
    }

    Font font = LoadFontEx(
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        40,
        codepoints,
        65536
    );

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTextEx(font,text,(Vector2){100,100},40,2,BLACK);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
}