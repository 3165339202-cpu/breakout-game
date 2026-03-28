#include "M06_ShadowEntity.h"
#include "M05_ShadowRecorder.h"
#include "raylib.h"
#include <vector>
#include <algorithm>
#include <cstdio>   // 新增

static std::vector<Shadow> shadows;

void SpawnShadow(Vector2 pos) {
    printf("Spawn shadow at (%f, %f)\n", pos.x, pos.y); // 调试输出
    Shadow s;
    s.type = EntityType::SHADOW;
    s.position = pos;
    s.size = {30, 30};
    s.lifetime = 3.0f;
    s.active = true;
    shadows.push_back(s);
}

void UpdateShadows(float dt) {
    for (auto& s : shadows) {
        s.lifetime -= dt;
        if (s.lifetime <= 0) {
            s.active = false;
            continue;
        }
        s.position = GetPastPosition(3.0f - s.lifetime);
    }
    shadows.erase(std::remove_if(shadows.begin(), shadows.end(),
        [](const Shadow& s){ return !s.active; }), shadows.end());
}

void DrawShadows() {
    for (auto& s : shadows) {
        if (s.active) {
            DrawRectangleV(s.position, s.size, GREEN); // 纯绿色，不透明
        }
    }
}