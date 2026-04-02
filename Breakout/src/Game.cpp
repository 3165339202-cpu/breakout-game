#include "Game.h"
#include <cmath>

Game::Game()
    : ball({400, 530}, {0, 0}, 10),
      paddle(340, 550, 120, 15) {}

void Game::Init() {
    score = 0;
    lives = 3;
    gameTime = 0.0f;
    currentState = GameState::MENU;

    bricks.clear();   // ⭐ 建议加（防止重复生成）

    Color colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE};

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 8; col++) {
            int randType = rand() % 10;

            BrickType type = NORMAL;
            if (randType == 0) type = EXPLOSIVE;
            else if (randType == 1) type = GOLDEN;

            bricks.emplace_back(
                50 + col * 95,
                80 + row * 35,
                85,
                25,
                colors[row],
                type
            );
        }
    }

    winCount = bricks.size();
}

void Game::Update() {

    switch (currentState) {

    // 🟢 开始界面
    case GameState::MENU:
        if (IsKeyPressed(KEY_SPACE)) {
            currentState = GameState::PLAYING;
        }
        break;

    // 🎮 游戏中
    case GameState::PLAYING:

        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PAUSED;
            break;
        }

        if (ball.IsLaunched()) gameTime += GetFrameTime();

        if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(18);
        if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(18);

        if (!ball.IsLaunched()) {
            ball.ResetToPaddle(
                paddle.GetRect().x + paddle.GetRect().width / 2,
                paddle.GetRect().y
            );

            if (IsKeyPressed(KEY_SPACE)) {
                ball.Launch(
                    paddle.GetRect().x + paddle.GetRect().width / 2,
                    paddle.GetRect().width
                );
            }
        }

        ball.ApplyGravity();
        ball.Move();
        ball.BounceEdge(800, 600);
        ball.BouncePaddle(paddle.GetRect());

        for (auto& brick : bricks) {
            if (brick.IsActive() &&
                ball.CheckBrickCollision(brick.GetRect())) {

                BrickType type = brick.GetType();
                Rectangle hit = brick.GetRect();

                brick.SetActive(false);

                if (type == GOLDEN) score += 50;
                else score += 10;

                winCount--;

                if (type == EXPLOSIVE) {
                    for (auto& other : bricks) {
                        if (!other.IsActive()) continue;

                        Rectangle r = other.GetRect();

                        if (fabs(r.x - hit.x) <= 100 &&
                            fabs(r.y - hit.y) <= 40) {
                            other.SetActive(false);
                            score += 5;
                            winCount--;
                        }
                    }
                }

                break;
            }
        }

        // ⭐ 状态切换（替代 gameOver）
        if (winCount <= 0) {
            currentState = GameState::VICTORY;
        }

        if (ball.GetPosition().y > 650) {
            lives--;
            if (lives <= 0) {
                currentState = GameState::GAMEOVER;
            } else {
                ball.ResetToPaddle(
                    paddle.GetRect().x + paddle.GetRect().width / 2,
                    paddle.GetRect().y
                );
            }
        }

        break;

    // ⏸️ 暂停
    case GameState::PAUSED:
        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PLAYING;
        }
        break;

    // 💀 失败
    case GameState::GAMEOVER:
        if (IsKeyPressed(KEY_R)) {
            Init();
            currentState = GameState::PLAYING;
        }
        break;

    // 🏆 胜利
    case GameState::VICTORY:
        if (IsKeyPressed(KEY_R)) {
            Init();
            currentState = GameState::PLAYING;
        }
        break;

    default:
        break;
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentState) {

    case GameState::MENU:
        DrawText("Press SPACE to Start", 250, 300, 20, WHITE);
        break;

    case GameState::PLAYING:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        ball.Draw();
        DrawText(TextFormat("Score: %d", score), 20, 20, 20, WHITE);
        break;

    case GameState::PAUSED:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        ball.Draw();
        DrawText("PAUSED", 350, 300, 30, YELLOW);
        break;

    case GameState::GAMEOVER:
        DrawText("GAME OVER", 300, 300, 30, RED);
        break;

    case GameState::VICTORY:
        DrawText("YOU WIN!", 300, 300, 30, GREEN);
        break;
    }

    EndDrawing();
}