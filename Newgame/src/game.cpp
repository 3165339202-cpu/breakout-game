/*#include "raylib.h"
#include <cmath>
int main() {
    // 初始化窗口    
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(800, 600, "Raylib Test");

Font font = LoadFontEx("font.ttf", 48, NULL, 0);    // 定义摄像机
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // 主游戏循环
    while (!WindowShouldClose()) {
        // 计算球体位置：绕Y轴旋转，半径5，速度可调
        float angle = GetTime() * 2.0f;          // 每秒旋转2弧度（约114度）
        float radius = 5.0f;
        Vector3 spherePos = {
            radius * sinf(angle),
            0.0f,
            radius * cosf(angle)
        };
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            // 绘制球体（实心+线框）
            DrawSphere(spherePos, 1.0f, RED);
            DrawSphereWires(spherePos, 1.0f, 16, 16, BLACK);
            // 绘制参考网格
            DrawGrid(10, 1.0f);
        EndMode3D();
	DrawTextEx(font,"公转的3D球体 - 使用Raylib",(Vector2){10,10},40,2,BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}*/
#include "raylib.h"
#include <cmath>
int main() {
    // 初始化窗口
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(800, 600, "Raylib Test");

Font font = LoadFontEx("font.ttf", 48, 0, 0);    // 定义摄像机
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // 主游戏循环
    while (!WindowShouldClose()) {
        // 计算球体位置：绕Y轴旋转，半径5，速度可调
        float angle = GetTime() * 2.0f;          // 每秒旋转2弧度（约114度）
        float radius = 5.0f;
        Vector3 spherePos = {
            radius * sinf(angle),
            0.0f,
            radius * cosf(angle)
        };
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            // 绘制球体（实心+线框）
            DrawSphere(spherePos, 1.0f, RED);
            DrawSphereWires(spherePos, 1.0f, 16, 16, BLACK);
            // 绘制参考网格
            DrawGrid(10, 1.0f);
        EndMode3D();
        DrawTextEx(font,"公转的3D球体 - 使用Raylib",(Vector2){10,10},40,2,BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
