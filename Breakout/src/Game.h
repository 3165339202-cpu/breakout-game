#pragma once
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

class Game {
private:
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;

    int score;
    int lives;
    int winCount;

    bool gameOver;
    bool paused;

    float gameTime;

public:
    Game();

    void Init();
    void Update();
    void Draw();
    void Shutdown();
};