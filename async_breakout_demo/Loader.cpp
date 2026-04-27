#include "Loader.h"

#include <chrono>
#include <thread>

bool Loader::StartLoading(int currentLevel) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (loading_) {
        return false;
    }

    loading_ = true;
    future_ = std::async(std::launch::async, [currentLevel]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        LevelData next;
        next.levelIndex = currentLevel + 1;
        next.backgroundTheme = next.levelIndex % 3;
        next.alternateLayout = (next.levelIndex % 2 == 0);
        return next;
    });

    return true;
}

bool Loader::IsLoading() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return loading_;
}

bool Loader::TryConsumeResult(LevelData& outLevel) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loading_) {
        return false;
    }

    if (future_.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        return false;
    }

    outLevel = future_.get();
    loading_ = false;
    return true;
}
