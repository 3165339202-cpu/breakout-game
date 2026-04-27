#include "Loader.h"

#include <chrono>
#include <random>
#include <thread>

Loader::Loader() : state(LoadState::IDLE), targetLevelNumber(1) {}

Loader::~Loader() {
    if (futureLevel.valid()) {
        futureLevel.wait();
    }
}

bool Loader::StartLoadingNextLevel() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state == LoadState::LOADING) {
        return false;
    }

    const int nextLevel = targetLevelNumber + 1;
    state = LoadState::LOADING;
    targetLevelNumber = nextLevel;
    completedLevel.reset();

    futureLevel = std::async(std::launch::async, [this, nextLevel]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return BuildLevelData(nextLevel);
    });

    return true;
}

void Loader::Update() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state != LoadState::LOADING || !futureLevel.valid()) {
        return;
    }

    const auto status = futureLevel.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
        completedLevel = futureLevel.get();
        state = LoadState::DONE;
    }
}

bool Loader::ConsumeLoadedLevel(LevelData& outLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    if (state != LoadState::DONE || !completedLevel.has_value()) {
        return false;
    }

    outLevel = completedLevel.value();
    completedLevel.reset();
    state = LoadState::IDLE;
    return true;
}

LoadState Loader::GetState() const {
    std::lock_guard<std::mutex> lock(mutex);
    return state;
}

int Loader::GetTargetLevelNumber() const {
    std::lock_guard<std::mutex> lock(mutex);
    return targetLevelNumber;
}

LevelData Loader::BuildLevelData(int levelNumber) const {
    LevelData level{};
    level.levelNumber = levelNumber;

    const Color backgrounds[] = {
        {20, 20, 35, 255},
        {18, 30, 46, 255},
        {34, 18, 42, 255},
        {16, 38, 22, 255}
    };
    level.background = backgrounds[levelNumber % 4];

    std::mt19937 rng(levelNumber * 1337u);
    std::uniform_int_distribution<int> colorIndex(0, 4);
    const Color palette[] = { RED, ORANGE, YELLOW, GREEN, SKYBLUE };

    const int rows = 4 + (levelNumber % 4);
    const int cols = 10;
    const float brickW = 106.0f;
    const float brickH = 28.0f;
    const float gap = 8.0f;
    const float startX = 70.0f;
    const float startY = 80.0f;

    level.bricks.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            BrickData b{};
            b.rect = {
                startX + c * (brickW + gap),
                startY + r * (brickH + gap),
                brickW,
                brickH
            };
            b.color = palette[colorIndex(rng)];
            b.active = true;
            level.bricks.push_back(b);
        }
    }

    return level;
}
