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
    struct ScoreEntry {
        std::string name;
        int score;
    };

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
    void Draw();
    void DrawLoadingUI() const;
    void DrawGameStatusUI() const;
    void DrawLeaderboard() const;

    void ResetBallOnPaddle();
    void ApplyLevel(const LevelData& level);
    void ResetRunState();
    Texture2D GetOrCreateTexture(const std::string& key, Color color);
    LevelData BuildInitialLevel() const;
    void LoadLeaderboard();
    void SaveLeaderboard() const;
    void AddLeaderboardScore(const std::string& name, int value);

    int screenWidth;
    int screenHeight;

    Paddle paddle;
    Ball ball;
    bool ballLaunched;

    Loader loader;
    std::vector<BrickData> bricks;
    int aliveBricks;
    int score;
    int lives;
    int currentLevel;
    Color background;
    std::string lastLoadNote;
    bool paused;
    bool gameOver;
    bool showLeaderboard;
    std::vector<ScoreEntry> leaderboard;

    std::unordered_map<std::string, Texture2D> textureCache;
};

#endif
