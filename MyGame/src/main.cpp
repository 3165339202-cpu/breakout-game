/*#include "raylib.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "My First C++ Game");

    SetTargetFPS(60);
    Vector2 playerPos = { screenWidth / 2.0f, screenHeight / 2.0f };
    float playerSpeed = 200.0f;
    float playerSize = 50.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) playerPos.x += playerSpeed * dt;
        if (IsKeyDown(KEY_LEFT))  playerPos.x -= playerSpeed * dt;
        if (IsKeyDown(KEY_UP))    playerPos.y -= playerSpeed * dt;
        if (IsKeyDown(KEY_DOWN))  playerPos.y += playerSpeed * dt;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangleV(playerPos, {playerSize, playerSize},RED);
        DrawText("Use arrow keys to move the red square", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}*/
#include "raylib.h"
#include "rlgl.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Rotating Cube Game");
    SetTargetFPS(60);

    // 摄像机
    Camera3D camera = { 0 };
    camera.position = (Vector3){4.0f, 4.0f, 4.0f};
    camera.target   = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up       = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy     = 45.0f;
    // camera.type = CAMERA_PERSPECTIVE;  // 删除这行

    float cubeRotation = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        cubeRotation += 20.0f * dt; // 每秒旋转 90 度
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // 绘制旋转立方体
	rlPushMatrix();
	rlRotatef(cubeRotation,0,1,0);
	DrawSphere(Vector3{0.0f, 0.0f, 0.0f}, 1.5f, RED);
	DrawCubeWires(Vector3{0.0f, 0.0f, 0.0f}, 1.5f, 1.5f, 1.5f, BLACK);

	rlPopMatrix();
        EndMode3D();

        DrawText("Rotating Cube Demo", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
