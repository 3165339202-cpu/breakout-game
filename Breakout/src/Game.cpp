#include "Game.h"
#include <cmath>

Game::Game()
    : ball({400, 530}, {0, 0}, 10),
      paddle(340, 550, 120, 15) {}

void Game::Init() {
    score = 0;
    lives = 3;
    gameOver = false;
    paused = false;
    gameTime = 0.0f;
    currentState = GameState::MENU;

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
        case GameState::PLAYING:
            if (IsKeyPressed(KEY_P)) paused = !paused;

            if (paused || gameOver) break;

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

                    // ⭐ 正确加分
                    if (type == GOLDEN) score += 50;
                    else score += 10;

                    winCount--;

                    // 爆炸砖
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

            if (winCount <= 0) gameOver = true;
            break;
        
        default:
            break;
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    for (auto& brick : bricks) brick.Draw();
    paddle.Draw();
    ball.Draw();

    DrawText(TextFormat("Score: %d", score), 20, 20, 20, WHITE);

    if (paused) DrawText("PAUSED", 350, 300, 30, YELLOW);
    if (gameOver) DrawText("GAME OVER", 300, 300, 30, RED);

    EndDrawing();
}

void Game::Shutdown() {
}