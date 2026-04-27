#include "Game.h"

#include <algorithm>
#include <cmath>
#include <string>

Game::Game() {
    InitWindow(kScreenWidth, kScreenHeight, "Raylib Async Loading Demo");
    SetTargetFPS(60);

    paddle_ = {kScreenWidth * 0.5f - 70.0f, kScreenHeight - 55.0f, 140.0f, 18.0f};
    ballPos_ = {kScreenWidth * 0.5f, kScreenHeight * 0.45f};
    ballVel_ = {220.0f, 180.0f};

    BuildBricks(false);
}

void Game::Run() {
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        Update(dt);

        BeginDrawing();
        Draw();
        EndDrawing();
    }

    CloseWindow();
}

void Game::Update(float dt) {
    if (IsKeyDown(KEY_LEFT)) {
        paddle_.x -= 520.0f * dt;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        paddle_.x += 520.0f * dt;
    }
    paddle_.x = std::clamp(paddle_.x, 0.0f, kScreenWidth - paddle_.width);

    ballPos_.x += ballVel_.x * dt;
    ballPos_.y += ballVel_.y * dt;

    if (ballPos_.x < ballRadius_ || ballPos_.x > kScreenWidth - ballRadius_) {
        ballVel_.x *= -1.0f;
    }
    if (ballPos_.y < ballRadius_ || ballPos_.y > kScreenHeight - ballRadius_) {
        ballVel_.y *= -1.0f;
    }

    const Rectangle ballRect{ballPos_.x - ballRadius_, ballPos_.y - ballRadius_, ballRadius_ * 2, ballRadius_ * 2};
    if (CheckCollisionRecs(ballRect, paddle_)) {
        ballVel_.y = -std::fabs(ballVel_.y);
    }

    if (IsKeyPressed(KEY_L)) {
        StartAsyncLoad();
    }

    loadingAnimTimer_ += dt;

    LevelData loadedData;
    if (loader_.TryConsumeResult(loadedData)) {
        ApplyLevelData(loadedData);
        loadState_ = LoadState::DONE;
    }

    if (loadState_ == LoadState::DONE && IsKeyPressed(KEY_ENTER)) {
        loadState_ = LoadState::IDLE;
    }
}

void Game::Draw() const {
    ClearBackground(BackgroundForTheme(backgroundTheme_));

    for (const Brick& brick : bricks_) {
        if (brick.active) {
            DrawRectangleRec(brick.rect, brick.color);
            DrawRectangleLinesEx(brick.rect, 1.0f, Fade(BLACK, 0.35f));
        }
    }

    DrawRectangleRec(paddle_, SKYBLUE);
    DrawCircleV(ballPos_, ballRadius_, RAYWHITE);

    DrawText(TextFormat("Level: %d", levelIndex_), 20, 20, 28, RAYWHITE);
    DrawText("Move paddle: LEFT/RIGHT", 20, 56, 22, Fade(RAYWHITE, 0.9f));
    DrawText("Press L: Async load next level", 20, 84, 22, Fade(RAYWHITE, 0.9f));
    DrawFPS(kScreenWidth - 100, 12);

    if (loadState_ == LoadState::LOADING && loader_.IsLoading()) {
        const float alphaWave = 0.45f + 0.55f * std::sin(loadingAnimTimer_ * 6.0f);
        const Color loadingColor = Fade(YELLOW, alphaWave);

        DrawText("Loading...", kScreenWidth / 2 - 95, kScreenHeight / 2 - 30, 44, loadingColor);

        const Vector2 center{(float)kScreenWidth / 2 + 130.0f, (float)kScreenHeight / 2 - 8.0f};
        DrawPoly(center, 3, 18.0f, loadingAnimTimer_ * 280.0f, ORANGE);

        DrawText("Main thread still rendering and accepting input", kScreenWidth / 2 - 260, kScreenHeight / 2 + 28, 20,
                 Fade(RAYWHITE, 0.9f));
    }

    if (loadState_ == LoadState::DONE) {
        DrawText("New level loaded! Press ENTER to clear this notice.", kScreenWidth / 2 - 260, kScreenHeight / 2 + 62,
                 22, GREEN);
    }
}

void Game::StartAsyncLoad() {
    if (loadState_ == LoadState::LOADING) {
        return;
    }

    if (loader_.StartLoading(levelIndex_)) {
        loadState_ = LoadState::LOADING;
    }
}

void Game::ApplyLevelData(const LevelData& data) {
    levelIndex_ = data.levelIndex;
    backgroundTheme_ = data.backgroundTheme;
    BuildBricks(data.alternateLayout);
}

void Game::BuildBricks(bool alternateLayout) {
    bricks_.clear();

    const int rows = alternateLayout ? 5 : 3;
    const int cols = alternateLayout ? 12 : 9;
    const float marginX = 70.0f;
    const float startY = 120.0f;
    const float gap = 8.0f;

    const float totalGap = gap * (cols - 1);
    const float brickW = (kScreenWidth - marginX * 2 - totalGap) / cols;
    const float brickH = 28.0f;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Brick brick;
            brick.rect = {marginX + c * (brickW + gap), startY + r * (brickH + gap), brickW, brickH};

            if (alternateLayout) {
                brick.color = (r % 2 == 0) ? VIOLET : PINK;
            } else {
                brick.color = (c % 2 == 0) ? ORANGE : GOLD;
            }

            bricks_.push_back(brick);
        }
    }
}

Color Game::BackgroundForTheme(int theme) const {
    switch (theme % 3) {
        case 0:
            return {24, 33, 58, 255};
        case 1:
            return {16, 56, 46, 255};
        default:
            return {64, 30, 30, 255};
    }
}
