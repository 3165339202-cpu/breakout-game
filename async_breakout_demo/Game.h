#pragma once

#include "Loader.h"

#include <raylib.h>
#include <vector>

enum class LoadState {
    IDLE,
    LOADING,
    DONE
};

struct Brick {
    Rectangle rect{};
    bool active = true;
    Color color = ORANGE;
};

class Game {
public:
    Game();
    void Run();

private:
    void Update(float dt);
    void Draw() const;
    void StartAsyncLoad();
    void ApplyLevelData(const LevelData& data);
    void BuildBricks(bool alternateLayout);
    Color BackgroundForTheme(int theme) const;

private:
    static constexpr int kScreenWidth = 1280;
    static constexpr int kScreenHeight = 720;

    Rectangle paddle_{};
    Vector2 ballPos_{};
    Vector2 ballVel_{};
    float ballRadius_ = 9.0f;

    std::vector<Brick> bricks_;

    LoadState loadState_ = LoadState::IDLE;
    Loader loader_;

    int levelIndex_ = 1;
    int backgroundTheme_ = 0;
    float loadingAnimTimer_ = 0.0f;
};
