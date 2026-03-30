#include "Ball.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

Ball::Ball(Vector2 pos, Vector2 sp, float r) {
    position = pos;
    velocity = sp;
    radius = r;
    gravity = 0.08f;
    maxSpeed = 15.0f;
    bounceForce = 0.5f;
    launched = false;
    launchCooldown = 0.0f;
    
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(nullptr));
        seeded = true;
    }
}

void Ball::Move() {
    if (!launched) return;
    position.x += velocity.x;
    position.y += velocity.y;
}

void Ball::Draw() {
    DrawCircleGradient((int)position.x, (int)position.y, radius + 3, Fade(ORANGE, 0.3f), RED);
    DrawCircleV(position, radius, RED);
    
    if (launched) {
        Vector2 endPos = { position.x + velocity.x * 2, position.y + velocity.y * 2 };
        DrawLineEx(position, endPos, 2, YELLOW);
    } else {
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("PRESS SPACE", (int)position.x - 55, (int)position.y - 30, 16, YELLOW);
        }
    }
}

void Ball::Launch(float paddleX, float paddleWidth) {
    if (launched) return;
    
    float angle = (rand() % 61 + 60) * 3.14159f / 180.0f;
    if (rand() % 2 == 0) angle = 3.14159f - angle;
    
    float baseSpeed = 8.0f;
    float speedVariation = (rand() % 5 - 2) * 0.5f;
    float launchSpeed = baseSpeed + speedVariation;
    
    velocity.x = launchSpeed * std::cos(angle);
    velocity.y = -launchSpeed * std::abs(std::sin(angle));
    
    launched = true;
    position.x = paddleX;
    position.y = 550 - radius - 5;
}

void Ball::ResetToPaddle(float paddleX, float paddleY) {
    position.x = paddleX;
    position.y = paddleY - radius - 5;
    velocity = {0, 0};
    launched = false;
}

void Ball::Reset(Vector2 pos, Vector2 sp) {
    position = pos;
    velocity = sp;
}

void Ball::AddBounceForce(float force) {
    velocity.y -= force;
}

void Ball::ApplyGravity() {
    if (!launched) return;
    velocity.y += gravity;
    
    float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (currentSpeed > maxSpeed) {
        velocity.x = (velocity.x / currentSpeed) * maxSpeed;
        velocity.y = (velocity.y / currentSpeed) * maxSpeed;
    }
}

void Ball::BounceEdge(int screenWidth, int screenHeight) {
    if (!launched) return;
    
    if (position.x - radius <= 5) {
        position.x = radius + 5;
        velocity.x = std::abs(velocity.x);
    }
    if (position.x + radius >= screenWidth - 5) {
        position.x = screenWidth - radius - 5;
        velocity.x = -std::abs(velocity.x);
    }
    if (position.y - radius <= 5) {
        position.y = radius + 5;
        velocity.y = std::abs(velocity.y);
        velocity.y += bounceForce;
    }
}

void Ball::BouncePaddle(Rectangle paddleRect) {
    if (!launched) return;
    if (velocity.y <= 0) return;
    
    if (position.y + radius >= paddleRect.y &&
        position.y + radius <= paddleRect.y + paddleRect.height + std::abs(velocity.y) &&
        position.x >= paddleRect.x - radius &&
        position.x <= paddleRect.x + paddleRect.width + radius) {
        
        float hitPoint = (position.x - (paddleRect.x + paddleRect.width / 2.0f)) / (paddleRect.width / 2.0f);
        hitPoint = std::clamp(hitPoint, -1.0f, 1.0f);
        
        float speedMagnitude = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        speedMagnitude = std::max(speedMagnitude + bounceForce * 2, 7.0f);
        
        float angle = 90.0f - hitPoint * 50.0f;
        float angleRad = angle * 3.14159f / 180.0f;
        
        velocity.x = speedMagnitude * std::cos(angleRad);
        velocity.y = -speedMagnitude * std::abs(std::sin(angleRad));
        position.y = paddleRect.y - radius;
    }
}

bool Ball::CheckBrickCollision(Rectangle brickRect) {
    if (!launched) return false;
    
    float closestX = std::max(brickRect.x, std::min(position.x, brickRect.x + brickRect.width));
    float closestY = std::max(brickRect.y, std::min(position.y, brickRect.y + brickRect.height));
    
    float distX = position.x - closestX;
    float distY = position.y - closestY;
    float distance = std::sqrt(distX * distX + distY * distY);
    
    if (distance < radius) {
        float distLeft = position.x - brickRect.x;
        float distRight = brickRect.x + brickRect.width - position.x;
        float distTop = position.y - brickRect.y;
        float distBottom = brickRect.y + brickRect.height - position.y;
        
        float minDistX = std::min(distLeft, distRight);
        float minDistY = std::min(distTop, distBottom);
        
        if (minDistX < minDistY) {
            velocity.x *= -1;
            position.x = (distLeft < distRight) ? brickRect.x - radius : brickRect.x + brickRect.width + radius;
        } else {
            velocity.y *= -1;
            if (distTop > distBottom) velocity.y -= bounceForce;
            position.y = (distTop < distBottom) ? brickRect.y - radius : brickRect.y + brickRect.height + radius;
        }
        return true;
    }
    return false;
}
