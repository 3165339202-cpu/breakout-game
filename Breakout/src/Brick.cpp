#include "Brick.h"
#include<cmath>
Brick::Brick(float x, float y, float width, float height, Color c, BrickType t)
    : GameObject({x, y}),
      VisualObject({x, y}, c, true)
{
    rect = { x, y, width, height };
    active = true;
    type = t;
    
    if (t == GOLDEN) scoreValue = 50;
    else if (t == EXPLOSIVE) scoreValue = 10;
    else scoreValue = 10;
}

void Brick::Draw() {
    if (active && visible) {
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 1, WHITE);

        if (type == EXPLOSIVE) {
            DrawText("X", rect.x + rect.width / 2 - 5, rect.y + 2, 20, WHITE);
        }

        if (type == GOLDEN) {
            DrawText("$", rect.x + rect.width / 2 - 5, rect.y + 2, 20, YELLOW);
        }
    }
}