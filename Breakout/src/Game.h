#pragma once
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

// ⭐ 状态机
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER,
    VICTORY,
    LEADERBOARD
};

class Game {
private:
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;

    int score;
    int lives;
    int winCount;

    float gameTime;

public:
    Game();

    GameState currentState;   // ⭐ 核心变量

    void Init();
    void Update();
    void Draw();
    void Shutdown();
};