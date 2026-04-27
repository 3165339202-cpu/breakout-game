#include "Game.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>

Game::Game()
    : screenWidth(1280),
      screenHeight(720),
      paddle{{540.0f, 670.0f, 200.0f, 20.0f}, 620.0f},
      ball{{640.0f, 650.0f}, {280.0f, -320.0f}, 10.0f},
      ballLaunched(false),
      aliveBricks(0),
      score(0),
      lives(3),
      currentLevel(1),
      background({20, 20, 35, 255}),
      lastLoadNote("resource warmup pending"),
      paused(false),
      gameOver(false),
      showLeaderboard(false) {
    LoadLeaderboard();
}

Game::~Game() {
    for (auto& [_, tex] : textureCache) {
        UnloadTexture(tex);
    }
}

void Game::Run() {
    InitWindow(screenWidth, screenHeight, "Raylib Breakout - Async Loading Demo");
    SetTargetFPS(60);

    ApplyLevel(BuildInitialLevel());

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        HandleInput(dt);
        Update(dt);

        BeginDrawing();
        Draw();
        EndDrawing();
    }

    CloseWindow();
}

void Game::HandleInput(float dt) {
    if (IsKeyPressed(KEY_P) && !gameOver) {
        paused = !paused;
    }
    if (IsKeyPressed(KEY_R)) {
        ResetRunState();
    }
    if (IsKeyPressed(KEY_B)) {
        showLeaderboard = !showLeaderboard;
    }

    if (paused || gameOver || showLeaderboard) {
        return;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        paddle.rect.x -= paddle.speed * dt;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        paddle.rect.x += paddle.speed * dt;
    }

    if (paddle.rect.x < 0) paddle.rect.x = 0;
    if (paddle.rect.x + paddle.rect.width > screenWidth) {
        paddle.rect.x = screenWidth - paddle.rect.width;
    }

    if (IsKeyPressed(KEY_SPACE) && !ballLaunched) {
        ballLaunched = true;
    }

    if (IsKeyPressed(KEY_L)) {
        loader.StartLoadingNextLevel();
    }
}

void Game::Update(float dt) {
    loader.Update();

    LevelData loadedLevel;
    if (loader.ConsumeLoadedLevel(loadedLevel)) {
        ApplyLevel(loadedLevel);
    }

    if (paused || gameOver || showLeaderboard) {
        return;
    }

    if (!ballLaunched) {
        ResetBallOnPaddle();
    } else {
        UpdateBall(dt);
        HandleCollisions();
    }

    if (aliveBricks <= 0 && loader.GetState() == LoadState::IDLE) {
        loader.StartLoadingNextLevel();
    }
}

void Game::UpdateBall(float dt) {
    ball.pos.x += ball.vel.x * dt;
    ball.pos.y += ball.vel.y * dt;

    if (ball.pos.x - ball.radius <= 0) {
        ball.pos.x = ball.radius;
        ball.vel.x *= -1;
    }
    if (ball.pos.x + ball.radius >= screenWidth) {
        ball.pos.x = static_cast<float>(screenWidth) - ball.radius;
        ball.vel.x *= -1;
    }
    if (ball.pos.y - ball.radius <= 0) {
        ball.pos.y = ball.radius;
        ball.vel.y *= -1;
    }

    if (ball.pos.y > screenHeight + 30.0f) {
        ballLaunched = false;
        lives--;
        if (lives <= 0) {
            gameOver = true;
            AddLeaderboardScore("Player", score);
        }
    }
}

void Game::HandleCollisions() {
    if (CheckCollisionCircleRec(ball.pos, ball.radius, paddle.rect) && ball.vel.y > 0) {
        const float hit = (ball.pos.x - paddle.rect.x) / paddle.rect.width - 0.5f;
        ball.vel.x = 560.0f * hit;
        ball.vel.y = -std::fabs(ball.vel.y);
    }

    for (auto& brick : bricks) {
        if (!brick.active) continue;

        if (CheckCollisionCircleRec(ball.pos, ball.radius, brick.rect)) {
            brick.active = false;
            aliveBricks--;
            score += 10;

            const float centerX = brick.rect.x + brick.rect.width / 2.0f;
            const float centerY = brick.rect.y + brick.rect.height / 2.0f;
            const float dx = ball.pos.x - centerX;
            const float dy = ball.pos.y - centerY;
            if (std::fabs(dx) > std::fabs(dy)) {
                ball.vel.x *= -1;
            } else {
                ball.vel.y *= -1;
            }
            break;
        }
    }

    if (aliveBricks <= 0) {
        score += 100;
    }
}

void Game::Draw() {
    ClearBackground(background);

    DrawRectangle(0, 0, screenWidth, 130, Fade(BLACK, 0.25f));
    DrawLine(0, 130, screenWidth, 130, Fade(WHITE, 0.2f));

    DrawFPS(14, 10);
    DrawText(TextFormat("Score: %d", score), 14, 38, 26, WHITE);
    DrawText(TextFormat("Level: %d", currentLevel), 260, 38, 26, WHITE);
    DrawText(TextFormat("Lives: %d", lives), 430, 38, 26, WHITE);
    DrawText("Press L to async load next level", 14, 72, 24, LIGHTGRAY);
    DrawText("P Pause  R Restart  B Leaderboard", 14, 100, 20, Fade(WHITE, 0.75f));

    for (const auto& brick : bricks) {
        if (!brick.active) continue;
        DrawRectangleRec(brick.rect, brick.color);
        DrawRectangleLinesEx(brick.rect, 1.0f, Fade(WHITE, 0.65f));
    }

    const Texture2D paddleTex = (currentLevel % 2 == 0)
                                    ? GetOrCreateTexture("paddle_even", BLUE)
                                    : GetOrCreateTexture("paddle_odd", GREEN);
    DrawTexturePro(
        paddleTex,
        Rectangle{0, 0, static_cast<float>(paddleTex.width), static_cast<float>(paddleTex.height)},
        paddle.rect,
        Vector2{0, 0},
        0,
        WHITE
    );
    DrawRectangleLinesEx(paddle.rect, 2.0f, WHITE);

    DrawCircleV(ball.pos, ball.radius, ORANGE);

    if (loader.GetState() == LoadState::LOADING) {
        DrawLoadingUI();
    }

    if (!ballLaunched) {
        DrawText("Press SPACE to launch ball", screenWidth / 2 - 160, screenHeight - 90, 28, YELLOW);
    }

    DrawGameStatusUI();

    if (showLeaderboard) {
        DrawLeaderboard();
    }
}

void Game::DrawLoadingUI() const {
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.35f));

    const float t = static_cast<float>(GetTime());
    const unsigned char alpha = static_cast<unsigned char>(160 + std::sin(t * 6.0f) * 80);
    DrawText(
        "Loading...",
        screenWidth / 2 - 90,
        screenHeight / 2 - 30,
        42,
        Color{255, 255, 255, alpha}
    );

    const Vector2 center{static_cast<float>(screenWidth / 2), static_cast<float>(screenHeight / 2 + 60)};
    const float angle = t * 220.0f;
    DrawPolyLinesEx(center, 6, 30.0f, angle, 4.0f, SKYBLUE);

    DrawText(
        TextFormat("Preparing Level %d", loader.GetTargetLevelNumber()),
        screenWidth / 2 - 120,
        screenHeight / 2 + 105,
        24,
        LIGHTGRAY
    );
}

void Game::ResetBallOnPaddle() {
    ball.pos.x = paddle.rect.x + paddle.rect.width / 2.0f;
    ball.pos.y = paddle.rect.y - ball.radius - 2.0f;
}

void Game::ApplyLevel(const LevelData& level) {
    currentLevel = level.levelNumber;
    background = level.background;
    bricks = level.bricks;
    lastLoadNote = level.loadNote;
    aliveBricks = static_cast<int>(bricks.size());

    paddle.rect.width = 200.0f - (currentLevel % 5) * 18.0f;
    if (paddle.rect.width < 120.0f) paddle.rect.width = 120.0f;
    paddle.rect.x = (screenWidth - paddle.rect.width) / 2.0f;

    ball.vel = {280.0f + currentLevel * 10.0f, -320.0f - currentLevel * 8.0f};
    ballLaunched = false;
    ResetBallOnPaddle();
}

void Game::ResetRunState() {
    score = 0;
    lives = 3;
    paused = false;
    gameOver = false;
    showLeaderboard = false;
    ApplyLevel(BuildInitialLevel());
}

Texture2D Game::GetOrCreateTexture(const std::string& key, Color color) {
    auto it = textureCache.find(key);
    if (it != textureCache.end()) {
        return it->second;
    }

    Image img = GenImageColor(4, 4, color);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    textureCache[key] = tex;
    return tex;
}

LevelData Game::BuildInitialLevel() const {
    LevelData level{};
    level.levelNumber = 1;
    level.background = {20, 20, 35, 255};
    level.loadNote = "initial level loaded on main thread";

    const Color rowColors[] = { RED, ORANGE, YELLOW, GREEN, BLUE };
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 10; ++c) {
            BrickData brick{};
            brick.rect = {70.0f + c * 114.0f, 170.0f + r * 36.0f, 106.0f, 28.0f};
            brick.color = rowColors[r];
            brick.active = true;
            level.bricks.push_back(brick);
        }
    }

    return level;
}

void Game::DrawGameStatusUI() const {
    if (paused) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.45f));
        DrawText("PAUSED", screenWidth / 2 - 90, screenHeight / 2 - 50, 56, YELLOW);
        DrawText("Press P to resume", screenWidth / 2 - 120, screenHeight / 2 + 20, 28, WHITE);
    }

    if (gameOver) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.65f));
        DrawText("GAME OVER", screenWidth / 2 - 160, screenHeight / 2 - 60, 62, RED);
        DrawText("Press R to restart", screenWidth / 2 - 135, screenHeight / 2 + 20, 30, WHITE);
    }
}

void Game::DrawLeaderboard() const {
    DrawRectangle(120, 120, screenWidth - 240, screenHeight - 240, Fade(BLACK, 0.88f));
    DrawRectangleLines(120, 120, screenWidth - 240, screenHeight - 240, GOLD);
    DrawText("LEADERBOARD", screenWidth / 2 - 160, 155, 44, GOLD);

    if (leaderboard.empty()) {
        DrawText("No records yet.", screenWidth / 2 - 120, 260, 30, LIGHTGRAY);
    } else {
        for (size_t i = 0; i < leaderboard.size(); ++i) {
            const auto& entry = leaderboard[i];
            DrawText(TextFormat("%2i. %-10s %6i", static_cast<int>(i + 1), entry.name.c_str(), entry.score),
                     220, 240 + static_cast<int>(i) * 40, 30, WHITE);
        }
    }

    DrawText("Press B to close", screenWidth / 2 - 105, screenHeight - 165, 28, LIGHTGRAY);
}

void Game::LoadLeaderboard() {
    leaderboard.clear();
    std::ifstream in("scores.txt");
    if (!in) return;

    std::string name;
    int value = 0;
    while (in >> name >> value) {
        leaderboard.push_back({name, value});
    }
    std::sort(leaderboard.begin(), leaderboard.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
    });
    if (leaderboard.size() > 10) leaderboard.resize(10);
}

void Game::SaveLeaderboard() const {
    std::ofstream out("scores.txt", std::ios::trunc);
    for (const auto& entry : leaderboard) {
        out << entry.name << " " << entry.score << "\n";
    }
}

void Game::AddLeaderboardScore(const std::string& name, int value) {
    leaderboard.push_back({name, value});
    std::sort(leaderboard.begin(), leaderboard.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
    });
    if (leaderboard.size() > 10) leaderboard.resize(10);
    SaveLeaderboard();
}
