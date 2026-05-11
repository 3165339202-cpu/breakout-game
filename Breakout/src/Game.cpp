#include "Game.h"
#include <cmath>
#include "json.hpp"
#include "PowerUpFactory.h"
#include "Particle.h"
#include "Brick.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "ExtendPaddleEffect.h"
#include "MultiBallEffect.h"
#include "SlowBallEffect.h"
using json = nlohmann::json;

namespace {
void DrawCenteredText(Font font, bool hasFont, const char* text, float y, float fontSize, Color color) {
    if (hasFont) {
        const Vector2 size = MeasureTextEx(font, text, fontSize, 1.0f);
        DrawTextEx(font, text, Vector2{(800.0f - size.x) * 0.5f, y}, fontSize, 1.0f, color);
    } else {
        const int size = static_cast<int>(fontSize);
        DrawText(text, (800 - MeasureText(text, size)) / 2, static_cast<int>(y), size, color);
    }
}

std::unique_ptr<PowerUpEffect> MakePowerUpEffect(PowerUpType type, const json& config) {
    const json powerups = config.contains("powerups") && config["powerups"].is_object()
                            ? config["powerups"]
                            : json::object();

    switch(type) {
        case PowerUpType::PADDLE_EXTEND: {
            const json settings = powerups.value("paddle_extend", json::object());
            return std::make_unique<ExtendPaddleEffect>(
                settings.value("extra_width", 40.0f),
                settings.value("duration", 5.0f));
        }
        case PowerUpType::MULTI_BALL: {
            const json settings = powerups.value("multi_ball", json::object());
            return std::make_unique<MultiBallEffect>(
                settings.value("extra_balls", 2));
        }
        case PowerUpType::SLOW_BALL: {
            const json settings = powerups.value("slow_ball", json::object());
            return std::make_unique<SlowBallEffect>(
                settings.value("speed_factor", 0.7f),
                settings.value("duration", 5.0f));
        }
    }

    return nullptr;
}
}

Game::Game()
    : paddle(340, 550, 120, 15),
      activeParticleCount(0),
      droppedParticleCount(0),
      leaderboard("scores.txt"),
      networkRole(NetworkRole::OFFLINE),
      remoteMoveLeft(false),
      remoteMoveRight(false),
      networkHint("单机模式"),
      uiFont{},
      hasChineseFont(false) {}

void Game::Init() {
    config = {
        {"powerups", {
            {"paddle_extend", {{"extra_width", 40}, {"duration", 5.0f}, {"drop_rate", 0.3f}}},
            {"multi_ball", {{"extra_balls", 2}, {"duration", 0.0f}, {"drop_rate", 0.2f}}},
            {"slow_ball", {{"speed_factor", 0.7f}, {"duration", 5.0f}, {"drop_rate", 0.25f}}}
        }}
    };

    const char* configCandidates[] = {
        "config.json",
        "src/config.json",
        "../src/config.json",
        "Breakout/src/config.json"
    };
    for (const char* path : configCandidates) {
        std::ifstream configFile(path);
        if (configFile.is_open()) {
            configFile >> config;
            TraceLog(LOG_INFO, "CONFIG: Loaded %s", path);
            break;
        }
    }

    score = 0;
    lives = 3;
    gameTime = 0.0f;
    scoreSaved = false;
    activeParticleCount = 0;
    droppedParticleCount = 0;
    for (auto& particle : particlePool) {
        particle.active = false;
        particle.life = 0.0f;
    }
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
        const int chineseCodepoints[] = {
            // 菜单、联网提示和 HUD 中会出现的中文。缺字会被 raylib 显示为 '?'，
            // 所以这里显式补齐“格、模、客、户、榜”等之前漏掉的字形。
            20002, 20027, 20301, 20572, 20837, 21019, 21040, 21147, 21152, 21333,
            21387, 21475, 21521, 22238, 22359, 22495, 22833, 22987, 23376, 23458,
            23545, 23616, 24050, 24314, 24320, 24323, 24335, 24453, 25103, 25143,
            25151, 25171, 25353, 25490, 25509, 26041, 26242, 26426, 26597, 26684,
            27036, 27169, 27744, 27979, 28216, 30475, 30742, 31354, 31471, 31561,
            31890, 32593, 34892, 35797, 35937, 36133, 36830, 36864, 38190, 38388,
            65292
        };

        std::vector<int> codepoints;
        codepoints.reserve(95 + (sizeof(chineseCodepoints) / sizeof(chineseCodepoints[0])));
        for (int c = 32; c <= 126; ++c) {
            codepoints.push_back(c); // ASCII for H/J/L/A/D and symbols
        }
        for (int c : chineseCodepoints) {
            codepoints.push_back(c);
        }

        const char* fontCandidates[] = {
            "fonts/NotoSansSC.otf",
            "../fonts/NotoSansSC.otf",
            "Breakout/fonts/NotoSansSC.otf"
        };
        for (const char* path : fontCandidates) {
            if (FileExists(path)) {
                uiFont = LoadFontEx(path, 32, codepoints.data(), static_cast<int>(codepoints.size()));
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
    // 兼容旧接口：以前这里会 insert 到 std::vector，可能触发扩容和大量复制。
    // 现在只把外部传入的粒子状态拷贝到对象池中的 inactive 槽位。
    for (const Particle& particle : newParticles) {
        SpawnParticle(particle.position, particle.velocity, particle.color, particle.life, particle.size);
    }
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
            RunParticleStressTest();
        }
        if (IsKeyPressed(KEY_O)) {
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
                        auto effect = MakePowerUpEffect(puType, config);
                        if (effect) {
                            powerUps.emplace_back(
                                Vector2{brick.GetRect().x + brick.GetRect().width/2, brick.GetRect().y},
                                puType,
                                std::move(effect)
                            );
                        }
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
        if (IsKeyPressed(KEY_O)) {
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

    UpdateParticles(GetFrameTime());
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
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "打砖块游戏" : "BREAKOUT GAME", 190, 44, WHITE);
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "按 空格 键开始单机游戏" : "Press SPACE to Start (Offline)", 285, 28, GREEN);
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "按 H 键创建局域网房间" : "Press H to Host LAN", 325, 28, SKYBLUE);
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "按 J 键加入 127.0.0.1 房间" : "Press J to Join LAN (127.0.0.1)", 365, 28, SKYBLUE);
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "房主: 方向键+空格  客户端: A/D" : "Host: arrows + SPACE; Client: A/D", 405, 24, LIGHTGRAY);
        DrawCenteredText(uiFont, hasChineseFont, hasChineseFont ? "按 L 键查看排行榜" : "Press L for Leaderboard", 445, 28, YELLOW);
        break;

    case GameState::PLAYING:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        DrawParticles();
        for (auto& pu : powerUps) pu.Draw();

        DrawRectangle(0, 0, 800, 64, Fade(BLACK, 0.78f));
        DrawFPS(10, 8);
        DrawText(TextFormat("Score:%d", score), 112, 10, 16, WHITE);
        DrawText(TextFormat("Lives:%d", lives), 220, 10, 16, WHITE);
        DrawText(TextFormat("Particles:%d/%d", activeParticleCount, MAX_PARTICLES), 320, 10, 16, WHITE);
        DrawText(TextFormat("FT:%.1fms", GetFrameTime() * 1000.0f), 505, 10, 16, LIGHTGRAY);
        DrawText(TextFormat("Pool:%.0f%%", GetParticlePoolUsage() * 100.0f), 635, 10, 16, LIGHTGRAY);
        if (hasChineseFont) {
            DrawTextEx(uiFont, networkHint.c_str(), Vector2{20, 36}, 18, 1, SKYBLUE);
        } else {
            DrawText(networkHint.c_str(), 20, 38, 16, SKYBLUE);
        }
        DrawText(TextFormat("Dropped:%d", droppedParticleCount), 180, 38, 16, ORANGE);
        DrawText("P:Stress", 300, 38, 16, SKYBLUE);
        DrawText("O:Pause", 400, 38, 16, SKYBLUE);
        break;

    case GameState::PAUSED:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        DrawParticles();
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
    const Vector2 pos = {
        brickRect.x + brickRect.width / 2.0f,
        brickRect.y + brickRect.height / 2.0f
    };
    SpawnParticleBurst(pos, color, 12, 0.8f, 3.0f);
}

bool Game::SpawnParticle(Vector2 position, Vector2 velocity, Color color, float life, float size) {
    // 性能优化报告：旧版粒子系统使用 std::vector<Particle>::emplace_back 每次爆炸动态追加粒子，
    // 并在每帧 update 后 erase/remove 已死亡粒子。压力测试下 vector 可能多次扩容、复制/移动元素，
    // erase 还会移动后续元素；如果改成 new/delete，堆分配也会因为分配器查找空闲块、同步和缓存未命中而变慢。
    // 对象池在 Init 时预留固定数组，运行时只扫描 inactive 粒子并 Reset，life<=0 仅 active=false，
    // 因此减少了粒子生成/销毁阶段的内存分配、释放、扩容和元素搬移，数据也更连续、更缓存友好。
    for (Particle& particle : particlePool) {
        if (!particle.active) {
            particle.Reset(position, velocity, color, life, size);
            return true;
        }
    }

    droppedParticleCount++;
    return false;
}

int Game::SpawnParticleBurst(Vector2 center, Color color, int count, float life, float size) {
    const double start = GetTime();
    int spawned = 0;

    for (int i = 0; i < count; i++) {
        Vector2 vel = {
            static_cast<float>(rand() % 100 - 50) * 0.5f,
            static_cast<float>(rand() % 100 - 50) * 0.5f
        };

        if (SpawnParticle(center, vel, color, life, size)) {
            spawned++;
        }
    }

    activeParticleCount = CountActiveParticles();
    TraceLog(LOG_INFO,
             "Particle pool burst: requested=%d spawned=%d active=%d capacity=%d dropped_total=%d spawn_time=%.3f ms",
             count, spawned, activeParticleCount, MAX_PARTICLES, droppedParticleCount,
             (GetTime() - start) * 1000.0);
    return spawned;
}

void Game::UpdateParticles(float dt) {
    activeParticleCount = 0;

    for (Particle& particle : particlePool) {
        if (!particle.active) continue;

        particle.Update(dt);
        if (particle.active) {
            activeParticleCount++;
        }
    }
}

void Game::DrawParticles() const {
    // raylib 会批处理相同绘制状态的简单图元；这里跳过 inactive 粒子，避免无意义 DrawCircleV 调用。
    for (const Particle& particle : particlePool) {
        if (particle.active) {
            particle.Draw();
        }
    }
}

int Game::CountActiveParticles() const {
    int count = 0;
    for (const Particle& particle : particlePool) {
        if (particle.active) count++;
    }
    return count;
}

float Game::GetParticlePoolUsage() const {
    return static_cast<float>(activeParticleCount) / static_cast<float>(MAX_PARTICLES);
}

void Game::RunParticleStressTest() {
    const Vector2 center = {400.0f, 300.0f};
    const int requestedParticles = MAX_PARTICLES;
    const int before = CountActiveParticles();
    const double start = GetTime();
    const int spawned = SpawnParticleBurst(center, GOLD, requestedParticles, 1.5f, 3.0f);
    const double elapsedMs = (GetTime() - start) * 1000.0;

    TraceLog(LOG_INFO,
             "KEY_P stress test: before=%d requested=%d spawned=%d after=%d fps=%d frame_time=%.3f ms elapsed=%.3f ms",
             before, requestedParticles, spawned, CountActiveParticles(), GetFPS(), GetFrameTime() * 1000.0f, elapsedMs);
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
