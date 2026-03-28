#include "M11_RenderBasic.h"
#include "raylib.h"

void DrawEntity(const Entity& e) {
    Color color;
    switch (e.type) {
        case EntityType::PLAYER: color = RED; break;
        case EntityType::SHADOW: color = {0, 255, 255, 128}; break;
        case EntityType::PLATFORM: color = GRAY; break;
        default: color = WHITE; break;
    }
    DrawRectangleV(e.position, e.size, color);
}
