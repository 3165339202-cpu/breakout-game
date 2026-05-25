#pragma once
#include <memory>
#include "json.hpp"

enum class PowerUpType { PADDLE_EXTEND, MULTI_BALL, SLOW_BALL };
class PowerUpEffect;

/**
 * @brief 根据道具类型与配置创建对应效果对象。
 *
 * 是什么：
 * - 一个工厂函数，返回 `PowerUpEffect` 的多态对象（`unique_ptr`）。
 *
 * 为什么：
 * - 将“类型分派 + 配置读取”集中，避免在游戏主逻辑中写大量 `switch`。
 *
 * 怎么用：
 * - 传入道具类型与 `config.json` 中的参数节点；
 * - 若配置缺省，函数内部或具体效果类会回退到默认值。
 */
std::unique_ptr<PowerUpEffect> CreatePowerUp(PowerUpType type, const nlohmann::json& config);
