#include "M05_ShadowRecorder.h"
#include <deque>

static std::deque<Vector2> positionHistory;
static int maxFrames = 180;

void InitRecorder(int max) {
    maxFrames = max;
    positionHistory.clear();
}

void RecordFrame(const Player& player) {
    positionHistory.push_back(player.position);
    if (positionHistory.size() > maxFrames) {
        positionHistory.pop_front();
    }
}

Vector2 GetPastPosition(float secondsAgo) {
    if (positionHistory.empty()) return {0,0};
    int index = (int)(secondsAgo * 60);
    if (index >= (int)positionHistory.size()) index = positionHistory.size() - 1;
    return positionHistory[positionHistory.size() - 1 - index];
}

bool GetPastOnGround(float secondsAgo) { return false; }