#include "Loader.h"

#include <chrono>
#include <fstream>
#include <random>
#include <sstream>
#include <thread>

Loader::Loader() : state(LoadState::IDLE), currentLevelNumber(1), targetLevelNumber(1) {}

Loader::~Loader() {
    if (workerFuture.valid()) {
        workerFuture.wait();
    }
}

bool Loader::StartLoadingNextLevel() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state == LoadState::LOADING) {
        return false;
    }

    if (workerFuture.valid()) {
        workerFuture.wait();
    }

    targetLevelNumber = currentLevelNumber + 1;
    const int nextLevel = targetLevelNumber;
    state = LoadState::LOADING;

    workerFuture = std::async(std::launch::async, [this, nextLevel]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        completedQueue.Push(BuildLevelData(nextLevel));
    });

    return true;
}

void Loader::Update() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state != LoadState::LOADING) {
        return;
    }

    if (workerFuture.valid() &&
        workerFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready &&
        !completedQueue.Empty()) {
        state = LoadState::DONE;
    }
}

bool Loader::ConsumeLoadedLevel(LevelData& outLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    if (state != LoadState::DONE) {
        return false;
    }

    const auto popped = completedQueue.TryPop();
    if (!popped.has_value()) {
        return false;
    }

    outLevel = popped.value();
    currentLevelNumber = outLevel.levelNumber;
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
    const float startY = 170.0f;

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

    level.loadNote = LoadRawResourceSample();
    return level;
}

std::string Loader::LoadRawResourceSample() const {
    const char* candidates[] = {"../raylib.h", "raylib.h", "../../raylib.h"};

    std::ifstream in;
    for (const char* path : candidates) {
        in.open(path, std::ios::binary);
        if (in) {
            break;
        }
        in.clear();
    }

    if (!in.is_open()) {
        return "resource sample unavailable (raylib.h not found)";
    }

    std::ostringstream oss;
    char buf[128]{};
    in.read(buf, sizeof(buf));
    const std::streamsize bytes = in.gcount();
    oss << "resource bytes=" << bytes;
    return oss.str();
}
