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


// ⭐ 状态机
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAMEOVER,
    VICTORY,
    LEADERBOARD
};

enum class NetworkRole {
    OFFLINE,
    HOST,
    CLIENT
};

            // 加载的配置          

class Game {
private:
    //Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;

    int score;
    int lives;
    int winCount;
    bool scoreSaved;

    // Game.h 中 private 部分添加：
    std::vector<Ball> balls;               // 支持多球
    std::vector<Particle> particles;    
    std::vector<PowerUp> powerUps;                                  // 道具列表
    nlohmann::json config;                                          // JSON 配置
    std::vector<std::unique_ptr<PowerUpEffect>> activeEffects;   // 粒子特效
    void GenerateBrickParticles(Rectangle brickRect, Color color);
    float gameTime;
    Leaderboard leaderboard;

    NetworkSession networkSession;
    NetworkRole networkRole;
    bool remoteMoveLeft;
    bool remoteMoveRight;
    std::string networkHint;

    std::string BuildNetworkState() const;
    void ApplyNetworkState(const std::string& state);

public:
    Game();

    GameState currentState;   // ⭐ 核心变量

    // Game.h 中 public 部分添加：

    // 访问器（给道具效果用）
    Paddle& GetPaddle() { return paddle; }
    // Game.h
    Ball& GetBall() { return balls.at(0); }  // 假设 balls 至少有一个球 // 返回主球（如果有）
    std::vector<Ball>& GetBalls() { return balls; }
    
    // 多球管理
    void AddBall(const Ball& newBall);
        void AddActiveEffect(std::unique_ptr<PowerUpEffect> eff) {
        activeEffects.push_back(std::move(eff));
    }
    // 粒子管理（后面会用到）
    void AddParticles(const std::vector<Particle>& newParticles);
    void Init();
    void Update();
    void Draw();
    void Shutdown(){};
    const std::string& GetNetworkHint() const { return networkHint; }
};