#ifndef LOADER_H
#define LOADER_H

#include <future>
#include <mutex>
#include <string>
#include <vector>

#include "ThreadSafeQueue.h"
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
    std::string loadNote;
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
    std::string LoadRawResourceSample() const;

    mutable std::mutex mutex;
    LoadState state;
    int currentLevelNumber;
    int targetLevelNumber;
    std::future<void> workerFuture;
    ThreadSafeQueue<LevelData> completedQueue;
};

#endif
