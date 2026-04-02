#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"
#include "GameObject.h"

class Paddle : public PhysicalObject, public VisualObject {
private:
    float width;
    float height;
    float screenWidth;

public:
    Paddle(float x, float y, float w, float h);

    void MoveLeft(float speed);
    void MoveRight(float speed);
    void Draw();

    Rectangle GetRect() {
        return { position.x, position.y, width, height };
    }
};

#endif
