#include "raylib.h"

int main() {
    // 初始化窗口
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Rotating Cube Game");

    SetTargetFPS(60);

    // 摄像机设置
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 4.0f, 4.0f, 4.0f }; // 摄像机位置
    camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f }; // 看向原点
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f }; // 上方向
    camera.fovy     = 45.0f;
    camera.type     = CAMERA_PERSPECTIVE;

    float cubeRotation = 0.0f; // 立方体旋转角度

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        cubeRotation += 90.0f * dt; // 每秒旋转 90 度

        // 渲染
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // 绘制旋转立方体
        DrawCube((Vector3){0, 0, 0}, 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires((Vector3){0, 0, 0}, 2.0f, 2.0f, 2.0f, MAROON);

        // 应用旋转
        rlPushMatrix();
        rlTranslatef(0, 0, 0);
        rlRotatef(cubeRotation, 0, 1, 0); // 绕Y轴旋转
        rlPopMatrix();

        EndMode3D();

        DrawText("Rotating Cube Demo", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
