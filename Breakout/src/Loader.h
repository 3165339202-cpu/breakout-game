#ifndef LOADER_H
#define LOADER_H

#include <future>
#include <mutex>
#include <optional>
#include <vector>
#include "raylib.h"

enum class LoadState {
    IDLE,
    LOADING,
    DONE
};

struct BrickData {
    Rectangle rect;
    Color color;
    bool active;
};

struct LevelData {
    int levelNumber;
    Color background;
    std::vector<BrickData> bricks;
};

class Loader {
public:
    Loader();
    ~Loader();

    bool StartLoadingNextLevel();
    void Update();

    bool ConsumeLoadedLevel(LevelData& outLevel);
    LoadState GetState() const;
    int GetTargetLevelNumber() const;

private:
    LevelData BuildLevelData(int levelNumber) const;

    mutable std::mutex mutex;
    LoadState state;
    int targetLevelNumber;
    std::future<LevelData> futureLevel;
    std::optional<LevelData> completedLevel;
};

#endif
