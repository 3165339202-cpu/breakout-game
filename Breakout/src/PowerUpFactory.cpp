#include "PowerUpFactory.h"
#include "ExtendPaddleEffect.h"
#include "MultiBallEffect.h"
#include "SlowBallEffect.h"

std::unique_ptr<PowerUpEffect> CreatePowerUp(PowerUpType type, const nlohmann::json& config) {
    switch(type) {
        case PowerUpType::PADDLE_EXTEND:
            return std::make_unique<ExtendPaddleEffect>(
                config["extra_width"].get<float>(),
                config["duration"].get<float>());
        case PowerUpType::MULTI_BALL:
            return std::make_unique<MultiBallEffect>(
                config["extra_balls"].get<int>());
        case PowerUpType::SLOW_BALL:
            return std::make_unique<SlowBallEffect>(
                config["speed_factor"].get<float>(),
                config["duration"].get<float>());
        default: return nullptr;
    }
}