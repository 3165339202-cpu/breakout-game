#include "Game.h"
#include <cmath>
#include "json.hpp"
#include "PowerUpFactory.h"
#include "Particle.h"
#include "Brick.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include "ExtendPaddleEffect.h"
#include "MultiBallEffect.h"
#include "SlowBallEffect.h"
using json = nlohmann::json;

namespace {
constexpr int kScreenWidth = 800;
constexpr int kScreenHeight = 600;
constexpr const char* kSaveFile = "breakout_save.json";
constexpr const char* kEditedLevelDir = "levels";

bool SameColor(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

void EnsureLevelDirectory() {
    mkdir(kEditedLevelDir, 0755);
}

void DrawCenteredText(const char* text, int y, float fontSize, float spacing, Color color, Font* font = nullptr) {
    float width = font ? MeasureTextEx(*font, text, fontSize, spacing).x
                       : static_cast<float>(MeasureText(text, static_cast<int>(fontSize)));
    float x = (kScreenWidth - width) * 0.5f;
    if (font) DrawTextEx(*font, text, Vector2{x, static_cast<float>(y)}, fontSize, spacing, color);
    else DrawText(text, static_cast<int>(x), y, static_cast<int>(fontSize), color);
}
}

Game::Game()
    : paddle(340, 550, 120, 15),
      score(0),
      lives(3),
      currentLevel(1),
      winCount(0),
      scoreSaved(false),
      gameTime(0.0f),
      leaderboard("scores.txt"),
      networkRole(NetworkRole::OFFLINE),
      remoteMoveLeft(false),
      remoteMoveRight(false),
      networkHint("单机模式"),
      uiFont{},
      hasChineseFont(false),
      levelFiles({"levels/level1.json", "levels/level2.json", "levels/level3.json"}),
      statusMessage(""),
      saveAvailable(false),
      loadedFromSave(false),
      editBrushType(NORMAL) {}

void Game::LoadConfig() {
    config = {
        {"powerups", {
            {"paddle_extend", {{"extra_width", 40}, {"duration", 5.0}, {"drop_rate", 0.3}}},
            {"multi_ball", {{"extra_balls", 2}, {"duration", 0.0}, {"drop_rate", 0.2}}},
            {"slow_ball", {{"speed_factor", 0.7}, {"duration", 5.0}, {"drop_rate", 0.25}}}
        }}
    };

    const char* candidates[] = {"config.json", "../src/config.json", "Breakout/src/config.json"};
    for (const char* path : candidates) {
        std::ifstream file(path);
        if (!file.is_open()) continue;
        try {
            json loaded;
            file >> loaded;
            if (!loaded.contains("powerups") || !loaded["powerups"].is_object()) {
                throw std::runtime_error("missing powerups object");
            }
            config.merge_patch(loaded);
            return;
        } catch (const std::exception& e) {
            statusMessage = std::string("配置文件错误，已使用默认配置: ") + e.what();
            return;
        }
    }
    statusMessage = "未找到 config.json，已使用默认配置。";
}

void Game::ResetTransientObjects() {
    balls.clear();
    particles.clear();
    powerUps.clear();
    activeEffects.clear();
}

void Game::ResetBallAndPaddle() {
    paddle.SetX(340);
    balls.clear();
    balls.emplace_back(Vector2{400, 530}, Vector2{0, 0}, 10);
}

Color Game::ColorFromName(const std::string& name) const {
    if (name == "red") return RED;
    if (name == "orange") return ORANGE;
    if (name == "yellow") return YELLOW;
    if (name == "green") return GREEN;
    if (name == "blue") return BLUE;
    if (name == "purple") return PURPLE;
    if (name == "gold") return GOLD;
    if (name == "pink") return PINK;
    return LIGHTGRAY;
}

std::string Game::ColorToName(Color color) const {
    if (SameColor(color, RED)) return "red";
    if (SameColor(color, ORANGE)) return "orange";
    if (SameColor(color, YELLOW)) return "yellow";
    if (SameColor(color, GREEN)) return "green";
    if (SameColor(color, BLUE)) return "blue";
    if (SameColor(color, PURPLE)) return "purple";
    if (SameColor(color, GOLD)) return "gold";
    if (SameColor(color, PINK)) return "pink";
    return "lightgray";
}

BrickType Game::BrickTypeFromName(const std::string& name) const {
    if (name == "explosive") return EXPLOSIVE;
    if (name == "golden") return GOLDEN;
    return NORMAL;
}

std::string Game::BrickTypeToName(BrickType type) const {
    if (type == EXPLOSIVE) return "explosive";
    if (type == GOLDEN) return "golden";
    return "normal";
}

void Game::RecalculateWinCount() {
    winCount = 0;
    for (const auto& brick : bricks) {
        if (brick.IsActive()) winCount++;
    }
}

bool Game::LoadLevelFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开关卡文件: " + path);
    }

    json level;
    file >> level;
    if (!level.contains("bricks") || !level["bricks"].is_array()) {
        throw std::runtime_error("关卡缺少 bricks 数组: " + path);
    }

    bricks.clear();
    for (const auto& item : level["bricks"]) {
        float x = item.at("x").get<float>();
        float y = item.at("y").get<float>();
        float width = item.value("width", 85.0f);
        float height = item.value("height", 25.0f);
        std::string colorName = item.value("color", "lightgray");
        std::string typeName = item.value("type", "normal");
        bricks.emplace_back(x, y, width, height, ColorFromName(colorName), BrickTypeFromName(typeName));
    }

    if (bricks.empty()) {
        throw std::runtime_error("关卡没有砖块: " + path);
    }
    RecalculateWinCount();
    return true;
}

void Game::LoadDefaultLevel(int levelNumber, const std::string& reason) {
    bricks.clear();
    Color colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE};
    int rows = 3 + std::min(levelNumber, 3);
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < 8; col++) {
            BrickType type = NORMAL;
            if (levelNumber >= 2 && (row + col) % 7 == 0) type = EXPLOSIVE;
            if (levelNumber >= 3 && (row * 3 + col) % 9 == 0) type = GOLDEN;
            bricks.emplace_back(50 + col * 95, 80 + row * 35, 85, 25, colors[row % 5], type);
        }
    }
    RecalculateWinCount();
    statusMessage = reason + " 已载入默认第 " + std::to_string(levelNumber) + " 关布局。";
}

bool Game::LoadLevel(int levelNumber) {
    currentLevel = std::max(1, levelNumber);
    std::string path = currentLevel <= static_cast<int>(levelFiles.size())
        ? levelFiles[currentLevel - 1]
        : levelFiles.back();

    try {
        LoadLevelFromFile(path);
        statusMessage = "已载入第 " + std::to_string(currentLevel) + " 关: " + path;
        return true;
    } catch (const std::exception& e) {
        LoadDefaultLevel(currentLevel, std::string("关卡 JSON 缺失或格式错误: ") + e.what());
        return false;
    }
}

bool Game::SaveExists() const {
    std::ifstream file(kSaveFile);
    return file.good();
}

bool Game::SaveGame() const {
    json save;
    save["score"] = score;
    save["lives"] = lives;
    save["currentLevel"] = currentLevel;
    save["bricks"] = json::array();
    for (const auto& brick : bricks) {
        Rectangle r = brick.GetRect();
        save["bricks"].push_back({
            {"x", r.x}, {"y", r.y}, {"width", r.width}, {"height", r.height},
            {"color", ColorToName(brick.GetColor())},
            {"type", BrickTypeToName(brick.GetType())},
            {"active", brick.IsActive()}
        });
    }

    std::ofstream out(kSaveFile);
    if (!out.is_open()) return false;
    out << save.dump(4);
    return true;
}

bool Game::LoadSaveGame() {
    std::ifstream file(kSaveFile);
    if (!file.is_open()) {
        statusMessage = "没有找到存档。";
        return false;
    }

    try {
        json save;
        file >> save;
        score = save.at("score").get<int>();
        lives = save.at("lives").get<int>();
        currentLevel = save.at("currentLevel").get<int>();
        LoadLevel(currentLevel);

        if (save.contains("bricks") && save["bricks"].is_array()) {
            bricks.clear();
            for (const auto& item : save["bricks"]) {
                float x = item.at("x").get<float>();
                float y = item.at("y").get<float>();
                float width = item.value("width", 85.0f);
                float height = item.value("height", 25.0f);
                Brick brick(x, y, width, height,
                           ColorFromName(item.value("color", "lightgray")),
                           BrickTypeFromName(item.value("type", "normal")));
                brick.SetActive(item.value("active", true));
                bricks.push_back(brick);
            }
        }
        ResetTransientObjects();
        ResetBallAndPaddle();
        RecalculateWinCount();
        scoreSaved = false;
        loadedFromSave = true;
        statusMessage = "已读取存档，继续第 " + std::to_string(currentLevel) + " 关。";
        return true;
    } catch (const std::exception& e) {
        statusMessage = std::string("存档格式错误，已忽略: ") + e.what();
        return false;
    }
}

void Game::DeleteSave() const {
    std::remove(kSaveFile);
}

void Game::LoadNextLevelOrWin() {
    currentLevel++;
    if (currentLevel > static_cast<int>(levelFiles.size())) {
        currentState = GameState::VICTORY;
        DeleteSave();
        return;
    }
    ResetTransientObjects();
    ResetBallAndPaddle();
    LoadLevel(currentLevel);
    SaveGame();
    currentState = GameState::PLAYING;
}

bool Game::SaveCurrentLayout() const {
    EnsureLevelDirectory();
    std::string path = currentLevel <= static_cast<int>(levelFiles.size())
        ? levelFiles[currentLevel - 1]
        : "levels/level" + std::to_string(currentLevel) + ".json";

    json layout;
    layout["name"] = "Edited Level " + std::to_string(currentLevel);
    layout["bricks"] = json::array();
    for (const auto& brick : bricks) {
        if (!brick.IsActive()) continue;
        Rectangle r = brick.GetRect();
        layout["bricks"].push_back({
            {"x", r.x}, {"y", r.y}, {"width", r.width}, {"height", r.height},
            {"color", ColorToName(brick.GetColor())},
            {"type", BrickTypeToName(brick.GetType())}
        });
    }

    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << layout.dump(4);
    return true;
}

void Game::HandleEditMode() {
    if (IsKeyPressed(KEY_ONE)) editBrushType = NORMAL;
    if (IsKeyPressed(KEY_TWO)) editBrushType = EXPLOSIVE;
    if (IsKeyPressed(KEY_THREE)) editBrushType = GOLDEN;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        auto hit = std::find_if(bricks.begin(), bricks.end(), [mouse](const Brick& brick) {
            return brick.IsActive() && CheckCollisionPointRec(mouse, brick.GetRect());
        });

        if (hit != bricks.end()) {
            bricks.erase(hit);
        } else if (mouse.y > 60 && mouse.y < 520) {
            float width = 85;
            float height = 25;
            float x = std::max(5.0f, std::min(710.0f, std::floor((mouse.x - 50) / 95.0f) * 95.0f + 50));
            float y = std::max(70.0f, std::min(500.0f, std::floor((mouse.y - 80) / 35.0f) * 35.0f + 80));
            Color color = editBrushType == GOLDEN ? GOLD : (editBrushType == EXPLOSIVE ? RED : SKYBLUE);
            bricks.emplace_back(x, y, width, height, color, editBrushType);
        }
        RecalculateWinCount();
    }

    if (IsKeyPressed(KEY_S)) {
        statusMessage = SaveCurrentLayout() ? "当前布局已保存为 JSON。" : "保存布局失败。";
    }

    if (IsKeyPressed(KEY_E)) {
        SaveCurrentLayout();
        currentState = GameState::PLAYING;
        statusMessage = "已退出编辑模式。";
    }
}

void Game::Init() {
    LoadConfig();
    score = 0;
    lives = 3;
    currentLevel = 1;
    gameTime = 0.0f;
    scoreSaved = false;
    loadedFromSave = false;
    saveAvailable = SaveExists();
    currentState = GameState::MENU;
    ResetTransientObjects();
    ResetBallAndPaddle();

    if (!networkSession.IsActive()) {
        networkRole = NetworkRole::OFFLINE;
        networkHint = "单机模式";
    }
    remoteMoveLeft = false;
    remoteMoveRight = false;

    if (!hasChineseFont) {
        const int chineseCodepoints[] = {
            // 常用汉字（菜单/状态/编辑模式）
            25171, 30742, 22359, 28216, 25103, 25353, 31354, 38190, 24320, 22987,
            21333, 26426, 21019, 24314, 23616, 22495, 32593, 25151, 38388, 21152,
            20837, 20027, 26041, 21521, 23458, 25490, 34892, 26524, 30475, 22238,
            22823, 31561, 32487, 23384, 26723, 20851, 32534, 36753, 20445, 35835,
            24403, 21069, 24067, 24050, 20445, 20026, 27809, 26377, 25214, 21040,
            35835, 21462, 35835, 38388, 26159, 36733, 25509, 35201, 21152, 36733,
            22833, 36133, 37329, 29983, 21629, 32493, 24674, 21457, 29616, 36793,
            23545, 25151, 38388, 25151, 23458, 25143, 26377, 29616, 25490, 34892,
            // 标点与全角符号
            65306, // ：
            65292, // ，
            12290, // 。
            65311, // ？
            12289, // 、
            65295  // ／
        };

        std::vector<int> codepoints;
        codepoints.reserve(95 + (sizeof(chineseCodepoints) / sizeof(chineseCodepoints[0])));
        for (int c = 32; c <= 126; ++c) codepoints.push_back(c);
        for (int c : chineseCodepoints) codepoints.push_back(c);

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

    LoadLevel(currentLevel);
}

void Game::AddBall(const Ball& newBall) {
    balls.push_back(newBall);
}

void Game::AddParticles(const std::vector<Particle>& newParticles) {
    particles.insert(particles.end(), newParticles.begin(), newParticles.end());
}

void Game::Update() {
    switch (currentState) {
    case GameState::LEADERBOARD:
        if (IsKeyPressed(KEY_L) || IsKeyPressed(KEY_ESCAPE)) currentState = GameState::MENU;
        break;

    case GameState::MENU:
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::LEADERBOARD;
            break;
        }
        if (saveAvailable && IsKeyPressed(KEY_C)) {
            if (LoadSaveGame()) currentState = GameState::PLAYING;
            saveAvailable = SaveExists();
        }
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_N)) {
            DeleteSave();
            saveAvailable = false;
            score = 0;
            lives = 3;
            currentLevel = 1;
            scoreSaved = false;
            ResetTransientObjects();
            ResetBallAndPaddle();
            LoadLevel(currentLevel);
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

    case GameState::EDITING:
        HandleEditMode();
        break;

    case GameState::PLAYING: {
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::LEADERBOARD;
            break;
        }
        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PAUSED;
            break;
        }
        if (IsKeyPressed(KEY_E)) {
            currentState = GameState::EDITING;
            statusMessage = "编辑模式：左键添加/删除，1普通 2爆炸 3金砖，S保存，E退出。";
            break;
        }
        if (IsKeyPressed(KEY_F5)) {
            statusMessage = SaveGame() ? "存档成功。" : "存档失败。";
            saveAvailable = SaveExists();
        }

        float dt = GetFrameTime();

        if (networkRole == NetworkRole::CLIENT) {
            bool sendLeft = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
            bool sendRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
            networkSession.SendInput(sendLeft, sendRight);

            std::string statePayload;
            if (networkSession.ReceiveState(statePayload)) ApplyNetworkState(statePayload);
            break;
        }

        if (networkRole == NetworkRole::HOST) {
            bool netLeft = false;
            bool netRight = false;
            if (networkSession.ReceiveInput(netLeft, netRight)) {
                remoteMoveLeft = netLeft;
                remoteMoveRight = netRight;
                if (networkSession.HasPeer()) networkHint = "局域网房主: 已连接 1 位客户端";
            }
        }

        if (!balls.empty() && balls[0].IsLaunched()) gameTime += dt;

        paddle.Update(dt);
        bool localLeft = IsKeyDown(KEY_LEFT);
        bool localRight = IsKeyDown(KEY_RIGHT);
        bool finalLeft = localLeft || (networkRole == NetworkRole::HOST && remoteMoveLeft);
        bool finalRight = localRight || (networkRole == NetworkRole::HOST && remoteMoveRight);

        if (finalLeft) paddle.MoveLeft(18);
        if (finalRight) paddle.MoveRight(18);

        for (auto& b : balls) {
            if (!b.IsLaunched()) {
                b.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width / 2, paddle.GetRect().y);
                break;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            for (auto& b : balls) {
                if (!b.IsLaunched()) {
                    b.Launch(paddle.GetRect().x + paddle.GetRect().width / 2, paddle.GetRect().width);
                    break;
                }
            }
        }

        for (auto& b : balls) {
            if (b.IsLaunched()) {
                b.ApplyGravity();
                b.Move();
                b.BounceEdge(kScreenWidth, kScreenHeight);
                b.BouncePaddle(paddle.GetRect());
            }
        }

        for (auto& pu : powerUps) pu.Update(dt);
        for (auto& pu : powerUps) {
            if (pu.active && pu.CheckCollision(paddle.GetRect())) pu.ApplyEffect(*this);
        }
        powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(),
                                      [](const PowerUp& pu) { return !pu.active || pu.position.y > 650; }),
                       powerUps.end());

        for (auto& effect : activeEffects) effect->Update(*this, dt);
        activeEffects.erase(std::remove_if(activeEffects.begin(), activeEffects.end(),
                                           [](const std::unique_ptr<PowerUpEffect>& e) { return e->IsExpired(); }),
                            activeEffects.end());

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
                                    config["powerups"]["paddle_extend"].value("extra_width", 40.0f),
                                    config["powerups"]["paddle_extend"].value("duration", 5.0f));
                                break;
                            case PowerUpType::MULTI_BALL:
                                effect = std::make_unique<MultiBallEffect>(
                                    config["powerups"]["multi_ball"].value("extra_balls", 2));
                                break;
                            case PowerUpType::SLOW_BALL:
                                effect = std::make_unique<SlowBallEffect>(
                                    config["powerups"]["slow_ball"].value("speed_factor", 0.7f),
                                    config["powerups"]["slow_ball"].value("duration", 5.0f));
                                break;
                        }
                        powerUps.emplace_back(Vector2{brick.GetRect().x + brick.GetRect().width/2, brick.GetRect().y},
                                             puType, std::move(effect));
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
            if (it->GetPosition().y > 650) it = balls.erase(it);
            else ++it;
        }

        if (balls.empty()) {
            lives--;
            SaveGame();
            if (lives <= 0) {
                currentState = GameState::GAMEOVER;
                DeleteSave();
            } else {
                balls.emplace_back(Vector2{400, 530}, Vector2{0, 0}, 10);
            }
        }

        if (winCount <= 0) LoadNextLevelOrWin();

        if (networkRole == NetworkRole::HOST && networkSession.IsActive()) networkSession.SendState(BuildNetworkState());
        break;
    }

    case GameState::PAUSED:
        if (IsKeyPressed(KEY_L)) currentState = GameState::LEADERBOARD;
        if (IsKeyPressed(KEY_P)) currentState = GameState::PLAYING;
        if (IsKeyPressed(KEY_F5)) statusMessage = SaveGame() ? "存档成功。" : "存档失败。";
        break;

    case GameState::GAMEOVER:
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::LEADERBOARD;
            break;
        }
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
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::LEADERBOARD;
            break;
        }
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
    for (auto& p : particles) p.Update(dt);
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& p) { return !p.IsAlive(); }),
                    particles.end());
}

void Game::Shutdown() {
    if (hasChineseFont && uiFont.texture.id > 0) UnloadFont(uiFont);
    hasChineseFont = false;
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentState) {
    case GameState::MENU:
        if (hasChineseFont) {
            DrawCenteredText("打砖块游戏", 165, 44, 1, WHITE, &uiFont);
            DrawCenteredText(saveAvailable ? "发现存档：按 C 继续，按 N/空格 开新游戏" : "按 空格 键开始单机游戏", 250, 25, 1, GREEN, &uiFont);
            DrawCenteredText("按 H 键创建局域网房间", 310, 28, 1, SKYBLUE, &uiFont);
            DrawCenteredText("按 J 键加入 127.0.0.1 房间", 350, 28, 1, SKYBLUE, &uiFont);
            DrawCenteredText("游戏中：F5存档  E编辑模式  L排行榜", 390, 24, 1, YELLOW, &uiFont);
        } else {
            DrawCenteredText("BREAKOUT GAME", 165, 30, 1, WHITE);
            DrawCenteredText(saveAvailable ? "Save found: C Continue, N/SPACE New Game" : "Press SPACE to Start (Offline)", 250, 20, 1, GREEN);
            DrawCenteredText("Press H to Host LAN", 310, 20, 1, SKYBLUE);
            DrawCenteredText("Press J to Join LAN (127.0.0.1)", 350, 20, 1, SKYBLUE);
            DrawCenteredText("In game: F5 Save, E Edit Mode, L Leaderboard", 390, 18, 1, YELLOW);
        }
        if (!statusMessage.empty()) {
            if (hasChineseFont) DrawTextEx(uiFont, statusMessage.c_str(), Vector2{20, 550}, 16, 1, ORANGE);
            else DrawText("Status updated", 20, 550, 16, ORANGE);
        }
        break;

    case GameState::PLAYING:
    case GameState::EDITING:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        for (auto& p : particles) p.Draw();
        for (auto& pu : powerUps) pu.Draw();

        DrawText(TextFormat("Score: %d", score), 20, 20, 20, WHITE);
        DrawText(TextFormat("Lives: %d", lives), 700, 20, 20, WHITE);
        DrawText(TextFormat("Level: %d/%d", currentLevel, static_cast<int>(levelFiles.size())), 350, 20, 20, WHITE);
        if (hasChineseFont) DrawTextEx(uiFont, networkHint.c_str(), Vector2{20, 50}, 22, 1, SKYBLUE);
        else DrawText("Offline mode", 20, 50, 18, SKYBLUE);
        DrawText("F5 Save | E Edit", 590, 50, 16, LIGHTGRAY);
        if (currentState == GameState::EDITING) {
            DrawRectangle(0, 0, kScreenWidth, 70, Fade(DARKBLUE, 0.85f));
            if (hasChineseFont) {
                DrawTextEx(uiFont, "编辑模式：左键添加/删除｜1 普通 2 爆炸 3 金砖｜S 保存 JSON｜E 退出",
                           Vector2{20, 20}, 24, 1, WHITE);
                DrawTextEx(uiFont, TextFormat("画笔：%s", BrickTypeToName(editBrushType).c_str()),
                           Vector2{20, 45}, 24, 1, YELLOW);
            } else {
                DrawText("EDIT MODE: Left click add/delete | 1 Normal 2 Explosive 3 Golden | S Save JSON | E Exit", 20, 20, 18, WHITE);
                DrawText(TextFormat("Brush: %s", BrickTypeToName(editBrushType).c_str()), 20, 45, 18, YELLOW);
            }
        }
        if (!statusMessage.empty()) {
            if (hasChineseFont) DrawTextEx(uiFont, statusMessage.c_str(), Vector2{20, 575}, 16, 1, ORANGE);
            else DrawText("Status updated", 20, 575, 16, ORANGE);
        }
        break;

    case GameState::PAUSED:
        for (auto& brick : bricks) brick.Draw();
        paddle.Draw();
        for (auto& b : balls) b.Draw();
        for (auto& p : particles) p.Draw();
        for (auto& pu : powerUps) pu.Draw();
        DrawText("PAUSED", 350, 285, 30, YELLOW);
        DrawText("P Resume | F5 Save", 300, 330, 20, WHITE);
        if (!statusMessage.empty()) {
            if (hasChineseFont) DrawTextEx(uiFont, statusMessage.c_str(), Vector2{20, 575}, 16, 1, ORANGE);
            else DrawText("Status updated", 20, 575, 16, ORANGE);
        }
        break;

    case GameState::GAMEOVER:
        DrawText("GAME OVER", 300, 250, 30, RED);
        DrawText("Press R to Restart", 260, 320, 20, WHITE);
        break;

    case GameState::VICTORY:
        DrawText("YOU WIN ALL LEVELS!", 240, 250, 30, GREEN);
        DrawText("Press R to Restart", 260, 320, 20, WHITE);
        break;

    case GameState::LEADERBOARD:
        DrawCenteredText("LEADERBOARD", 80, 30, 1, GOLD);
        for (int i = 0; i < leaderboard.GetCount(); i++) {
            ScoreEntry entry;
            if (leaderboard.GetEntry(i + 1, entry)) {
                DrawText(TextFormat("%d. %s - %d", i + 1, entry.name, entry.score), 250, 150 + i * 30, 20, WHITE);
            }
        }
        DrawCenteredText("Press L or ESC to return", 500, 20, 1, GRAY);
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
    for (const auto& brick : bricks) brickMask.push_back(brick.IsActive() ? '1' : '0');

    std::ostringstream oss;
    oss << static_cast<int>(currentState) << '|'
        << score << '|'
        << lives << '|'
        << winCount << '|'
        << currentLevel << '|'
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
    while (std::getline(ss, token, '|')) fields.push_back(token);

    if (fields.size() < 12) return;

    currentState = static_cast<GameState>(std::stoi(fields[0]));
    score = std::stoi(fields[1]);
    lives = std::stoi(fields[2]);
    winCount = std::stoi(fields[3]);
    int incomingLevel = std::stoi(fields[4]);
    if (incomingLevel != currentLevel) LoadLevel(incomingLevel);

    paddle.SetX(std::stof(fields[5]));

    if (balls.empty()) balls.emplace_back(Vector2{400, 530}, Vector2{0, 0}, 10);
    balls[0].Reset(Vector2{std::stof(fields[6]), std::stof(fields[7])},
                   Vector2{std::stof(fields[8]), std::stof(fields[9])});
    balls[0].SetLaunched(std::stoi(fields[10]) != 0);

    const std::string& brickMask = fields[11];
    size_t n = std::min(bricks.size(), brickMask.size());
    for (size_t i = 0; i < n; ++i) bricks[i].SetActive(brickMask[i] == '1');
}
