#include "raylib.h"
#include "M01_Core.h"
#include "M02_Input.h"
#include "M03_Physics.h"
#include "M04_PlayerState.h"
#include "M05_ShadowRecorder.h"
#include "M06_ShadowEntity.h"
#include "M07_MechanicLogic.h"
#include "M08_MechanicRender.h"
#include "M09_EnemyAI.h"
#include "M10_LevelLoader.h"
#include "M11_RenderBasic.h"
#include "M12_Animation.h"
#include "M13_Camera.h"
#include "M14_UI.h"
#include "M15_Audio.h"

#include <vector>
#include <memory>

// 全局实体列表定义
std::vector<std::unique_ptr<Entity>> g_entities;

// 重力常数（可被模块修改）
// float GRAVITY = 800.0f;  // 像素/秒²

// 主函数
int main() {
    InitCore();                     // 初始化窗口等

    InitUI();                       // 中文字体

    // 初始化各模块（调用各自的Init函数，若存在）
    InitRecorder(180);              // 3秒 * 60帧
    // InitPlayer(*(Player*)nullptr);   // 这里需要实际获取玩家实体，稍后完善
    InitCamera({0,0});

    LoadLevel("assets/level1.json"); // 加载第一关

    // 游戏主循环（由RunGameLoop实现，这里直接写）
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();   // 获取帧时间

        // 输入
        Action action = GetAction();

        // 逻辑更新
        // 1. 更新玩家状态
        // 需要找到玩家实体
        Player* player = nullptr;
        for (auto& e : g_entities) {
            if (e->type == EntityType::PLAYER) {
                player = (Player*)e.get();
                break;
            }
        }
        if (player) {
            UpdatePlayer(*player, action, dt);
            RecordFrame(*player);
            if (action == Action::SHADOW) {
                SpawnShadow(player->position);
            }
        }

        if (action == Action::SHADOW) {
            SpawnShadow(player->position);
        }

        // 2. 记录影子状态
        if (player) RecordFrame(*player);

        // 3. 更新影子实体
        UpdateShadows(dt);

        // 4. 更新机关逻辑
        UpdateMechanics(dt);

        // 5. 更新敌人AI
        UpdateEnemies(dt);

        // 6. 物理更新（处理重力、移动后碰撞）
        UpdatePhysics(dt);

        // 7. 更新摄像机
        if (player) UpdateCamera(player->position);

        // 8. 更新动画
        UpdateAnimations(dt);

        // 绘制
        BeginDrawing();
        ClearBackground(BLACK);

        // 获取摄像机偏移，用于绘制时平移
        Vector2 offset = GetCameraOffset();
        // 简单实现：直接绘制所有实体（不做摄像机变换，留待以后）
        for (auto& e : g_entities) {
            // 优先使用动画绘制，否则用基础绘制
            DrawAnimated(*e);
        }

        DrawMechanics();   // 机关可能单独绘制（或已包含在实体中，根据设计）

        DrawShadows();   // 绘制影子

        DrawUI();

        EndDrawing();
    }

    ShutdownCore();
    return 0;
}