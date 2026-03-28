#include "Brick.h"
#include<cmath>
Brick::Brick(float x, float y, float width, float height, Color c, BrickType t) {
    rect = { x, y, width, height };
    active = true;
    color = c;
    type = t;   // ⭐新增
}

void Brick::Draw() {
    if (active) {
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 1, WHITE);

        // 💥 爆炸砖显示 X
        if (type == EXPLOSIVE) {
            DrawText("X", rect.x + rect.width / 2 - 5, rect.y + 2, 20, WHITE);
        }
        if (type == GOLDEN){
            float t =GetTime();
            Color blinkColor = {255,215,0,255};
            blinkColor.r=200+55*sinf(t*5);
            blinkColor.g=150+105*sinf(t*5);
            blinkColor.b=0;
            DrawRectangleRec(rect, blinkColor);
            DrawRectangleLinesEx(rect, 2, GOLD);
        }
    }
}