#include "Game.h"
#include <cmath>
#include "json.hpp"
#include "PowerUpFactory.h"
#include "Particle.h"
#include "Brick.h"
#include <algorithm>
#include <fstream>   // 用于文件读取
#include "ExtendPaddleEffect.h"
#include "MultiBallEffect.h"
#include "SlowBallEffect.h"
using json = nlohmann::json;

Game::Game()
    : //ball({400, 530}, {0, 0}, 10),
      paddle(340, 550, 120, 15) ,
      leaderboard("scores.txt")
    {
    }

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

void Game::AddBall(const Ball& newBall) {
    balls.push_back(newBall);
}

void Game::Update() {
    if (IsKeyPressed(KEY_L)) {
    currentState = GameState::LEADERBOARD;
}
    switch (currentState) {

    // 🟢 开始界面
    case GameState::LEADERBOARD:
    if (IsKeyPressed(KEY_L)) {
        currentState = GameState::MENU;  // 返回主菜单
    }
    break;
    case GameState::MENU:
        if (IsKeyPressed(KEY_SPACE)) {
            currentState = GameState::PLAYING;
        }
        break;

    // 🎮 游戏中
   case GameState::PLAYING:
{
    if (IsKeyPressed(KEY_P)) {
        currentState = GameState::PAUSED;
        break;
    }

    float dt = GetFrameTime();
    if (!balls.empty() && balls[0].IsLaunched()) {
        gameTime += dt;  // 只要有一个球已发射就开始计时
    }

    // 板移动
        paddle.Update(dt);  // 更新板的计时器（如加长效果倒计时）
    if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(18);
    if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(18);

    // 处理未发射的球（绑定到板上）
    for (auto& b : balls) {
        if (!b.IsLaunched()) {
            b.ResetToPaddle(
                paddle.GetRect().x + paddle.GetRect().width / 2,
                paddle.GetRect().y
            );
            break;  // 只处理第一个未发射的球
        }
    }

    // 发射球
    if (IsKeyPressed(KEY_SPACE)) {
        for (auto& b : balls) {
            if (!b.IsLaunched()) {
                b.Launch(
                    paddle.GetRect().x + paddle.GetRect().width / 2,
                    paddle.GetRect().width
                );
                break;  // 一次只发射一个球
            }
        }
    }

    // 更新所有球的物理状态
    for (auto& b : balls) {
        if (b.IsLaunched()) {
            b.ApplyGravity();
            b.Move();
            b.BounceEdge(800, 600);
            b.BouncePaddle(paddle.GetRect());
        }
    }
        // 更新道具位置
    for (auto& pu : powerUps) {
        pu.Update(dt);
    }
    // 道具与板碰撞检测
    for (auto& pu : powerUps) {
        if (pu.active && pu.CheckCollision(paddle.GetRect())) {
            pu.ApplyEffect(*this);
        }
    }
    // 移除失效或超出屏幕的道具
    powerUps.erase(
        std::remove_if(powerUps.begin(), powerUps.end(),
                       [](const PowerUp& pu) { return !pu.active || pu.position.y > 650; }),
        powerUps.end()
    );
        // 更新持续效果
    for (auto& effect : activeEffects) {
        effect->Update(*this, dt);
    }
    // 移除过期效果
    activeEffects.erase(
        std::remove_if(activeEffects.begin(), activeEffects.end(),
                       [](const std::unique_ptr<PowerUpEffect>& e) { return e->IsExpired(); }),
        activeEffects.end()
    );

    // 碰撞检测：标记已处理的砖块，防止重复计分
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
                    // 随机选择道具类型
                    PowerUpType puType = static_cast<PowerUpType>(rand() % 3);
                    // 从配置中获取参数并创建效果对象
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
                    // 创建道具实体并加入列表
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

                brickProcessed[i] = true;  // 标记为已处理

                // 爆炸砖范围摧毁
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
                break;  // 砖块已摧毁，跳出内层球循环
            }
        }
    }

    // 处理出界的球（移除或扣命）
    for (auto it = balls.begin(); it != balls.end(); ) {
        if (it->GetPosition().y > 650) {
            it = balls.erase(it);  // 移除出界的球
        } else {
            ++it;
        }
    }

    // 如果没有球了，扣一条命并重置一个球
    if (balls.empty()) {
        lives--;
        if (lives <= 0) {
            currentState = GameState::GAMEOVER;
        } else {
            // 重新生成一个球并绑定到板上
            Ball newBall({400, 530}, {0, 0}, 10);
            balls.push_back(newBall);
        }
    }

    // 胜利判定
    if (winCount <= 0) {
        currentState = GameState::VICTORY;
    }

    break;
}
    // ⏸️ 暂停
    case GameState::PAUSED:
        if (IsKeyPressed(KEY_P)) {
            currentState = GameState::PLAYING;
        }
        break;

    // 💀 失败
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

    // 🏆 胜利
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
    // 移除生命结束的粒子
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return !p.IsAlive(); }),
        particles.end()
    );
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (currentState) {

    case GameState::MENU:
    DrawText("BREAKOUT GAME", 260, 200, 30, WHITE);
    DrawText("Press SPACE to Start", 250, 300, 20, GREEN);
    DrawText("Press L for Leaderboard", 230, 350, 20, YELLOW);
    break;

    /*case GameState::MENU:
        DrawText("Press SPACE to Start", 250, 300, 20, WHITE);
        break;*/

    case GameState::PLAYING:
    for (auto& brick : bricks) brick.Draw();
    paddle.Draw();
    for (auto& b : balls) b.Draw();
            for (auto& p : particles) p.Draw();
            for (auto& pu : powerUps) pu.Draw();

    DrawText(TextFormat("Score: %d", score), 20, 20, 20, WHITE);
    DrawText(TextFormat("Lives: %d", lives), 700, 20, 20, WHITE);
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