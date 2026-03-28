#include "M10_LevelLoader.h"
#include "M04_PlayerState.h"  
#include "Common.h"
#include <vector>
#include <memory>

bool LoadLevel(const char* filename) {
    // 基础版本手动创建几个实体
    g_entities.clear();

    // 创建玩家
    auto player = std::make_unique<Player>();
    InitPlayer(*player);
    player->position = {250, 520};   // 确保在平台上方
    g_entities.push_back(std::move(player));

    // 创建一个平台
    auto platform = std::make_unique<Entity>();
    platform->type = EntityType::PLATFORM;
    platform->position = {200, 550};
    platform->size = {200, 20};
    platform->active = true;
    g_entities.push_back(std::move(platform));

    return true;
}