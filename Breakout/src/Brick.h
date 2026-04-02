#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"
#include "GameObject.h"   


enum BrickType {
    NORMAL,
    EXPLOSIVE,
    GOLDEN
};

class Brick : public VisualObject {   
    Rectangle rect;
    bool active;
    BrickType type;
    int scoreValue;

public:
    
    Brick(float x, float y, float width, float height, Color c, BrickType t = NORMAL);

    void Draw();

    bool IsActive() { return active; }
    void SetActive(bool a) { active = a; }
    Rectangle GetRect() { return rect; }

    BrickType GetType() { return type; }
    int GetScoreValue() { return scoreValue; }
};

#endif