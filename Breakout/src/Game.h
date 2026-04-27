#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Loader.h"
#include "raylib.h"

class Game {
public:
    Game();
    ~Game();

    void Run();

private:
    struct Paddle {
        Rectangle rect;
        float speed;
    };

    struct Ball {
        Vector2 pos;
        Vector2 vel;
        float radius;
    };

    void HandleInput(float dt);
    void Update(float dt);
    void UpdateBall(float dt);
    void HandleCollisions();
    void Draw() const;
    void DrawLoadingUI() const;

    void ResetBallOnPaddle();
    void ApplyLevel(const LevelData& level);
    Texture2D GetOrCreateTexture(const std::string& key, Color color);
    LevelData BuildInitialLevel() const;

    int screenWidth;
    int screenHeight;

    Paddle paddle;
    Ball ball;
    bool ballLaunched;

    Loader loader;
    std::vector<BrickData> bricks;
    int aliveBricks;
    int score;
    int currentLevel;
    Color background;

    std::unordered_map<std::string, Texture2D> textureCache;
};

#endif
