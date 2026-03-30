#include "Paddle.h"

Paddle::Paddle(float x, float y, float w, float h)
    : GameObject({x, y}),
      PhysicalObject({x, y}, {0, 0}, 0),
      VisualObject({x, y}, WHITE, true)
{
    width = w;
    height = h;
    screenWidth = 800; // 你原来的窗口宽度
}

void Paddle::MoveLeft(float speed) {
    position.x -= speed;
    if (position.x < 5) position.x = 5;
}

void Paddle::MoveRight(float speed) {
    position.x += speed;
    if (position.x + width > screenWidth - 5)
        position.x = screenWidth - width - 5;
}

void Paddle::Draw() {
    if (visible) {
        DrawRectangle(position.x, position.y, width, height, color);
    }
}