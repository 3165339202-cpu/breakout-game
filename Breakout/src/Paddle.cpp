#include "Paddle.h"

Paddle::Paddle(float x, float y, float w, float h)
    : GameObject({x, y}),
      PhysicalObject({x, y}, {0, 0}, 0),
      VisualObject({x, y}, WHITE, true)
{
    width = w;
    height = h;
    originalWidth = w;
    effectTimer = 0.0f;
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
void Paddle::Extend(float extraWidth, float duration) {
    // 如果当前没有延长效果，记录原始宽度
    if (effectTimer <= 0) {
        originalWidth = width;
    }
    width = originalWidth + extraWidth;
    effectTimer = duration;
}

void Paddle::Update(float dt) {
    if (effectTimer > 0) {
        effectTimer -= dt;
        if (effectTimer <= 0) {
            width = originalWidth;   // 恢复原始宽度
        }
    }
}

void Paddle::Reset(float x, float y) {
    position = {x, y};
    width = originalWidth;
    effectTimer = 0.0f;
}
