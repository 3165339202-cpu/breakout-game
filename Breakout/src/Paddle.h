#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"
#include "GameObject.h"

class Paddle : public PhysicalObject, public VisualObject {
private:
    float width;
    float height;
    float screenWidth;
    float originalWidth;
    float effectTimer;

public:
    Paddle(float x, float y, float w, float h);

    void MoveLeft(float speed);
    void MoveRight(float speed);
    void Draw();
    void Extend(float extraWidth, float duration);
    void Update(float dt); 

    Rectangle GetRect() const {
        return { position.x, position.y, width, height };
    }

    void SetX(float x) { position.x = x; }
    float GetX() const { return position.x; }
};

#endif
