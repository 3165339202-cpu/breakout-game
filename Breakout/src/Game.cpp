#include "Game.h"
#include <cmath>
#include "json.hpp"
#include "PowerUpFactory.h"
#include "Particle.h"
#include "Brick.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include "ExtendPaddleEffect.h"
#include "MultiBallEffect.h"
#include "SlowBallEffect.h"
using json = nlohmann::json;

Game::Game()
    : paddle(340, 550, 120, 15),
      leaderboard("scores.txt"),
      networkRole(NetworkRole::OFFLINE),
      remoteMoveLeft(false),
      remoteMoveRight(false),
      networkHint("单机模式"),
      uiFont{},
      hasChineseFont(false) {}

void Game::Init() {
    std::ifstream configFile("config.json");
    if (configFile.is_open()) {
        configFile >> config;
    }
    score = 0;
    lives = 3;
    gameTime = 0.0f;
    scoreSaved = false;
    currentState = GameState::MENU;
    balls.clear();
    balls.emplace_back(Vector2{400, 530}, Vector2{0, 0}, 10);

    if (!networkSession.IsActive()) {
        networkRole = NetworkRole::OFFLINE;
        networkHint = "单机模式";
    }
    remoteMoveLeft = false;
    remoteMoveRight = false;

    if (!hasChineseFont) {
        const char* fontCandidates[] = {
            "fonts/NotoSansSC.otf",
            "../fonts/NotoSansSC.otf",
            "Breakout/fonts/NotoSansSC.otf"
        };
        for (const char* path : fontCandidates) {
            if (FileExists(path)) {
                uiFont = LoadFontEx(path, 32, nullptr, 0);
                if (uiFont.texture.id > 0) {
                    hasChineseFont = true;
                    break;
                }
            }
        }
    }

    bricks.clear();

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

void Game::AddBall(const Ball& newBall) {
    balls.push_back(newBall);
}

void Game::AddParticles(const std::vector<Particle>& newParticles) {
    particles.insert(particles.end(), newParticles.begin(), newParticles.end());
}

void Game::Update() {
    if (IsKeyPressed(KEY_L)) {
        currentState = GameState::LEADERBOARD;
    }

    switch (currentState) {
    case GameState::LEADERBOARD:
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::MENU;
        }
        break;

    case GameState::MENU:
        if (IsKeyPressed(KEY_SPACE)) {
            networkSession.Shutdown();
            networkRole = NetworkRole::OFFLINE;
            networkHint = "单机模式";
            currentState = GameState::PLAYING;
        }
        if (IsKeyPressed(KEY_H)) {
            networkSession.Shutdown();
            if (networkSession.StartHost(45200)) {
                networkRole = NetworkRole::HOST;
                networkHint = "局域网房主: 端口 45200，等待客户端";
                currentState = GameState::PLAYING;
            } else {
                networkRole = NetworkRole::OFFLINE;
                networkHint = "创建房间失败，回退单机模式";
            }
        }
        if (IsKeyPressed(KEY_J)) {
            networkSession.Shutdown();
            if (networkSession.StartClient("127.0.0.1", 45200)) {
                networkRole = NetworkRole::CLIENT;
                networkHint = "客户端: 已连接到 127.0.0.1:45200";
                currentState = GameState::PLAYING;
            } else {
                networkRole = NetworkRole::OFFLINE;
                networkHint = "加入房间失败，回退单机模式";
            }
        }
        break;

    case GameState::PLAYING: {
        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PAUSED;
            break;
        }

        float dt = GetFrameTime();

        if (networkRole == NetworkRole::CLIENT) {
            bool sendLeft = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
            bool sendRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
            networkSession.SendInput(sendLeft, sendRight);

            std::string statePayload;
            if (networkSession.ReceiveState(statePayload)) {
                ApplyNetworkState(statePayload);
            }
            break;
        }

        if (networkRole == NetworkRole::HOST) {
            bool netLeft = false;
            bool netRight = false;
            if (networkSession.ReceiveInput(netLeft, netRight)) {
                remoteMoveLeft = netLeft;
                remoteMoveRight = netRight;
                if (networkSession.HasPeer()) {
                    networkHint = "局域网房主: 已连接 1 位客户端";
                }
            }
        }

        if (!balls.empty() && balls[0].IsLaunched()) {
            gameTime += dt;
        }

        paddle.Update(dt);
        bool localLeft = IsKeyDown(KEY_LEFT);
        bool localRight = IsKeyDown(KEY_RIGHT);
        bool finalLeft = localLeft || (networkRole == NetworkRole::HOST && remoteMoveLeft);
        bool finalRight = localRight || (networkRole == NetworkRole::HOST && remoteMoveRight);

        if (finalLeft) paddle.MoveLeft(18);
        if (finalRight) paddle.MoveRight(18);

        for (auto& b : balls) {
            if (!b.IsLaunched()) {
                b.ResetToPaddle(
                    paddle.GetRect().x + paddle.GetRect().width / 2,
                    paddle.GetRect().y
                );
                break;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& b : balls) {
                if (!b.IsLaunched()) {
                    b.Launch(
                        paddle.GetRect().x + paddle.GetRect().width / 2,
                        paddle.GetRect().width
                    );
                    break;
                }
            }
        }

        for (auto& b : balls) {
            if (b.IsLaunched()) {
                b.ApplyGravity();
                b.Move();
                b.BounceEdge(800, 600);
                b.BouncePaddle(paddle.GetRect());
            }
        }

        for (auto& pu : powerUps) {
            pu.Update(dt);
        }

        for (auto& pu : powerUps) {
            if (pu.active && pu.CheckCollision(paddle.GetRect())) {
                pu.ApplyEffect(*this);
            }
        }

        powerUps.erase(
            std::remove_if(powerUps.begin(), powerUps.end(),
                           [](const PowerUp& pu) { return !pu.active || pu.position.y > 650; }),
            powerUps.end()
        );

        for (auto& effect : activeEffects) {
            effect->Update(*this, dt);
        }

        activeEffects.erase(
            std::remove_if(activeEffects.begin(), activeEffects.end(),
                           [](const std::unique_ptr<PowerUpEffect>& e) { return e->IsExpired(); }),
            activeEffects.end()
        );

        std::vector<bool> brickProcessed(bricks.size(), false);

        for (size_t i = 0; i < bricks.size(); ++i) {
            auto& brick = bricks[i];
            if (!brick.IsActive() || brickProcessed[i]) continue;

            for (auto& b : balls) {
                if (!b.IsLaunched()) continue;

                if (b.CheckBrickCollision(brick.GetRect())) {
                    BrickType type = brick.GetType();
                    Rectangle hit = brick.GetRect();
                    Color brickColor = brick.GetColor();

                    brick.SetActive(false);
                    if ((rand() % 100) < 30) {
                        PowerUpType puType = static_cast<PowerUpType>(rand() % 3);
                        std::unique_ptr<PowerUpEffect> effect;
                        switch(puType) {
                            case PowerUpType::PADDLE_EXTEND:
                                effect = std::make_unique<ExtendPaddleEffect>(
                                    config["powerups"]["paddle_extend"]["extra_width"],
                                    config["powerups"]["paddle_extend"]["duration"]);
                                break;
                            case PowerUpType::MULTI_BALL:
                                effect = std::make_unique<MultiBallEffect>(
                                    config["powerups"]["multi_ball"]["extra_balls"]);
                                break;
                            case PowerUpType::SLOW_BALL:
                                effect = std::make_unique<SlowBallEffect>(
                                    config["powerups"]["slow_ball"]["speed_factor"],
                                    config["powerups"]["slow_ball"]["duration"]);
                                break;
                        }
                        powerUps.emplace_back(
                            Vector2{brick.GetRect().x + brick.GetRect().width/2, brick.GetRect().y},
                            puType,
                            std::move(effect)
                        );
                    }
                    GenerateBrickParticles(hit, brickColor);

                    if (type == GOLDEN) score += 50;
                    else score += 10;
                    winCount--;

                    brickProcessed[i] = true;

                    if (type == EXPLOSIVE) {
                        for (size_t j = 0; j < bricks.size(); ++j) {
                            auto& other = bricks[j];
                            if (!other.IsActive() || brickProcessed[j]) continue;
                            Rectangle r = other.GetRect();
                            if (fabs(r.x - hit.x) <= 100 && fabs(r.y - hit.y) <= 40) {
                                other.SetActive(false);
                                GenerateBrickParticles(r, other.GetColor());
                                score += 5;
                                winCount--;
                                brickProcessed[j] = true;
                            }
                        }
                    }
                    break;
                }
            }
        }

        for (auto it = balls.begin(); it != balls.end(); ) {
            if (it->GetPosition().y > 650) {
                it = balls.erase(it);
            } else {
                ++it;
            }
        }

        if (balls.empty()) {
            lives--;
            if (lives <= 0) {
                currentState = GameState::GAMEOVER;
            } else {
                Ball newBall({400, 530}, {0, 0}, 10);
                balls.push_back(newBall);
            }
        }

        if (winCount <= 0) {
            currentState = GameState::VICTORY;
        }

        if (networkRole == NetworkRole::HOST && networkSession.IsActive()) {
            networkSession.SendState(BuildNetworkState());
        }

        break;
    }

    case GameState::PAUSED:
        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PLAYING;
        }
        break;

    case GameState::GAMEOVER:
        if (!scoreSaved) {
            leaderboard.AddScore("Player", score);
            scoreSaved = true;
        }

        if (IsKeyPressed(KEY_R)) {
            Init();
            currentState = GameState::PLAYING;
        }
        break;

    case GameState::VICTORY:
        if (!scoreSaved) {
            leaderboard.AddScore("Player", score);
            scoreSaved = true;
        }
        if (IsKeyPressed(KEY_R)) {
            Init();
            currentState = GameState::PLAYING;
        }
        break;

    default:
        break;
    }

    float dt = GetFrameTime();
    for (auto& p : particles) {
        p.Update(dt);
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return !p.IsAlive(); }),
        particles.end()
    );
}


void Game::Shutdown() {
    if (hasChineseFont && uiFont.texture.id > 0) {
        UnloadFont(uiFont);
    }
    hasChineseFont = false;
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentState) {
    case GameState::MENU:
        if (hasChineseFont) {
            DrawTextEx(uiFont, "打砖块游戏", Vector2{280, 200}, 44, 1, WHITE);
            DrawTextEx(uiFont, "按 空格 键开始单机游戏", Vector2{190, 285}, 28, 1, GREEN);
            DrawTextEx(uiFont, "按 H 键创建局域网房间", Vector2{185, 325}, 28, 1, SKYBLUE);
            DrawTextEx(uiFont, "按 J 键加入 127.0.0.1 房间", Vector2{165, 365}, 28, 1, SKYBLUE);
            DrawTextEx(uiFont, "房主: 方向键+空格  客户端: A/D", Vector2{145, 405}, 24, 1, LIGHTGRAY);
            DrawTextEx(uiFont, "按 L 键查看排行榜", Vector2{230, 445}, 28, 1, YELLOW);
        } else {
            DrawText("BREAKOUT GAME", 260, 200, 30, WHITE);
            DrawText("Press SPACE to Start (Offline)", 210, 290, 20, GREEN);
            DrawText("Press H to Host LAN", 260, 330, 20, SKYBLUE);
            DrawText("Press J to Join LAN (127.0.0.1)", 180, 360, 20, SKYBLUE);
            DrawText("Host: arrows + SPACE; Client: A/D", 190, 390, 18, LIGHTGRAY);
            DrawText("Press L for Leaderboard", 230, 430, 20, YELLOW);
        }
        break;

    case GameState::PLAYING:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        for (auto& p : particles) p.Draw();
        for (auto& pu : powerUps) pu.Draw();

        DrawText(TextFormat("Score: %d", score), 20, 20, 20, WHITE);
        DrawText(TextFormat("Lives: %d", lives), 700, 20, 20, WHITE);
        DrawText(networkHint.c_str(), 20, 50, 18, SKYBLUE);
        break;

    case GameState::PAUSED:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        for (auto& p : particles) p.Draw();
        for (auto& pu : powerUps) pu.Draw();

        DrawText("PAUSED", 350, 300, 30, YELLOW);
        break;

    case GameState::GAMEOVER:
        DrawText("GAME OVER", 300, 250, 30, RED);
        DrawText("Press R to Restart", 260, 320, 20, WHITE);
        break;

    case GameState::VICTORY:
        DrawText("YOU WIN!", 300, 250, 30, GREEN);
        DrawText("Press R to Restart", 260, 320, 20, WHITE);
        break;

    case GameState::LEADERBOARD:
        DrawText("LEADERBOARD", 300, 80, 30, GOLD);

        for (int i = 0; i < leaderboard.GetCount(); i++) {
            ScoreEntry entry;
            if (leaderboard.GetEntry(i + 1, entry)) {
                DrawText(
                    TextFormat("%d. %s - %d", i + 1, entry.name, entry.score),
                    250,
                    150 + i * 30,
                    20,
                    WHITE
                );
            }
        }

        DrawText("Press L to return", 270, 500, 20, GRAY);
        break;
    }

    EndDrawing();
}

void Game::GenerateBrickParticles(Rectangle brickRect, Color color) {
    for (int i = 0; i < 12; i++) {
        Vector2 vel = {
            (float)(rand() % 100 - 50) * 0.5f,
            (float)(rand() % 100 - 50) * 0.5f
        };
        Vector2 pos = {
            brickRect.x + brickRect.width / 2.0f,
            brickRect.y + brickRect.height / 2.0f
        };
        particles.emplace_back(pos, vel, color, 0.8f, 3.0f);
    }
}

std::string Game::BuildNetworkState() const {
    float paddleX = paddle.GetRect().x;

    Vector2 ballPos{400.0f, 530.0f};
    Vector2 ballVel{0.0f, 0.0f};
    int ballLaunched = 0;
    if (!balls.empty()) {
        ballPos = balls[0].GetPosition();
        ballVel = balls[0].GetSpeed();
        ballLaunched = balls[0].IsLaunched() ? 1 : 0;
    }

    std::string brickMask;
    brickMask.reserve(bricks.size());
    for (const auto& brick : bricks) {
        brickMask.push_back(brick.IsActive() ? '1' : '0');
    }

    std::ostringstream oss;
    oss << static_cast<int>(currentState) << '|'
        << score << '|'
        << lives << '|'
        << winCount << '|'
        << paddleX << '|'
        << ballPos.x << '|'
        << ballPos.y << '|'
        << ballVel.x << '|'
        << ballVel.y << '|'
        << ballLaunched << '|'
        << brickMask;
    return oss.str();
}

void Game::ApplyNetworkState(const std::string& state) {
    std::stringstream ss(state);
    std::string token;
    std::vector<std::string> fields;
    while (std::getline(ss, token, '|')) {
        fields.push_back(token);
    }

    if (fields.size() < 11) return;

    currentState = static_cast<GameState>(std::stoi(fields[0]));
    score = std::stoi(fields[1]);
    lives = std::stoi(fields[2]);
    winCount = std::stoi(fields[3]);

    paddle.SetX(std::stof(fields[4]));

    if (balls.empty()) {
        balls.emplace_back(Vector2{400, 530}, Vector2{0, 0}, 10);
    }
    balls[0].Reset(Vector2{std::stof(fields[5]), std::stof(fields[6])},
                   Vector2{std::stof(fields[7]), std::stof(fields[8])});
    balls[0].SetLaunched(std::stoi(fields[9]) != 0);

    const std::string& brickMask = fields[10];
    size_t n = std::min(bricks.size(), brickMask.size());
    for (size_t i = 0; i < n; ++i) {
        bricks[i].SetActive(brickMask[i] == '1');
    }
}
