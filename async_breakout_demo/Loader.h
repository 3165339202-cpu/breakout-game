#pragma once

#include <future>
#include <mutex>

struct LevelData {
    int levelIndex = 1;
    int backgroundTheme = 0;
    bool alternateLayout = false;
};

class Loader {
public:
    Loader() = default;
    ~Loader() = default;

    bool StartLoading(int currentLevel);
    bool IsLoading() const;
    bool TryConsumeResult(LevelData& outLevel);

private:
    mutable std::mutex mutex_;
    std::future<LevelData> future_;
    bool loading_ = false;
};
