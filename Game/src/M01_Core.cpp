#include "M01_Core.h"
#include "raylib.h"
#include "Common.h"
#include <vector>
#include <memory>

void InitCore() {
    InitWindow(800, 600, "Shadow Echo - 基础框架");
    SetTargetFPS(60);
}

void ShutdownCore() {
    CloseWindow();
}

void RunGameLoop() {
    // 主循环已在 main.cpp 中实现，此处留空
}