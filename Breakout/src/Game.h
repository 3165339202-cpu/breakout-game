#pragma once
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "PowerUp.h"
#include "PowerUpEffect.h"
#include "PowerUpFactory.h"
#include <vector>
#include<ctime>
#include "Leaderboard.h"
#include "Particle.h"
#include "json.hpp"
#include "NetworkSession.h"
#include <string>

// 游戏流程状态机。
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER,
    VICTORY,
    LEADERBOARD,
    EDITING
};

// 联机模式角色。
enum class NetworkRole {
    OFFLINE,
    HOST,
    CLIENT
};

/**
 * @brief 游戏主控制类，负责“状态、规则、渲染、联机”的统一编排。
 *
 * 是什么：
 * - `Game` 封装了完整一局游戏所需的核心对象（球、挡板、砖块、道具、排行榜、联机会话等）。
 *
 * 为什么：
 * - 将输入处理、物理更新、关卡读写、存档和渲染集中管理，避免逻辑分散在 `main` 或多个全局函数中。
 *
 * 怎么用：
 * - 创建实例后调用 `Init()` 初始化资源；
 * - 在主循环中每帧调用 `Update()` 和 `Draw()`；
 * - 退出前调用 `Shutdown()` 释放资源。
 */
class Game {
private:
    Paddle paddle;
    std::vector<Brick> bricks;

    int score;
    int lives;
    int currentLevel;
    int winCount;
    bool scoreSaved;

    std::vector<Ball> balls;
    std::vector<Particle> particles;
    std::vector<PowerUp> powerUps;
    nlohmann::json config;
    std::vector<std::unique_ptr<PowerUpEffect>> activeEffects;
    void GenerateBrickParticles(Rectangle brickRect, Color color);
    float gameTime;
    Leaderboard leaderboard;

    NetworkSession networkSession;
    NetworkRole networkRole;
    bool remoteMoveLeft;
    bool remoteMoveRight;
    std::string networkHint;

    Font uiFont;
    bool hasChineseFont;

    std::vector<std::string> levelFiles;
    std::string statusMessage;
    bool saveAvailable;
    bool loadedFromSave;
    BrickType editBrushType;

    std::string BuildNetworkState() const;
    void ApplyNetworkState(const std::string& state);
    void LoadConfig();
    void ResetTransientObjects();
    void ResetBallAndPaddle();
    bool LoadLevel(int levelNumber);
    bool LoadLevelFromFile(const std::string& path);
    void LoadDefaultLevel(int levelNumber, const std::string& reason);
    void RecalculateWinCount();
    bool SaveGame() const;
    bool LoadSaveGame();
    bool SaveExists() const;
    void DeleteSave() const;
    void LoadNextLevelOrWin();
    void HandleEditMode();
    bool SaveCurrentLayout() const;
    Color ColorFromName(const std::string& name) const;
    std::string ColorToName(Color color) const;
    BrickType BrickTypeFromName(const std::string& name) const;
    std::string BrickTypeToName(BrickType type) const;

public:
    Game();

    GameState currentState;

    // 访问器：用于道具效果在不暴露内部结构的情况下访问核心对象。
    Paddle& GetPaddle() { return paddle; }
    Ball& GetBall() { return balls.at(0); }
    std::vector<Ball>& GetBalls() { return balls; }

    // 多球管理：道具触发后将新球插入球列表。
    void AddBall(const Ball& newBall);
    void AddActiveEffect(std::unique_ptr<PowerUpEffect> eff) {
        activeEffects.push_back(std::move(eff));
    }

    // 粒子管理：统一收集外部生成的粒子，以便在 Update/Draw 中管理生命周期。
    void AddParticles(const std::vector<Particle>& newParticles);

    void Init();
    void Update();
    void Draw();
    void Shutdown();
    const std::string& GetNetworkHint() const { return networkHint; }
};
