// PowerUpFactory.h
#pragma once
#include <memory>
#include "json.hpp"
enum class PowerUpType { PADDLE_EXTEND, MULTI_BALL, SLOW_BALL };
class PowerUpEffect;

std::unique_ptr<PowerUpEffect> CreatePowerUp(PowerUpType type, const nlohmann::json& config);
    