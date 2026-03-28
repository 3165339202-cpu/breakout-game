#ifndef CORE_H
#define CORE_H

void InitCore();        // 初始化核心模块（创建窗口、加载配置等）
void ShutdownCore();    // 关闭核心模块
void RunGameLoop();     // 主循环（由main调用）

#endif