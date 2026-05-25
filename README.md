# Breakout Game（C++/raylib）

## 项目简介
这是一个基于 **C++17 + raylib** 实现的打砖块（Breakout）项目，主游戏代码位于 `Breakout/` 目录。项目包含：

- 基础打砖块玩法（小球、挡板、砖块、得分、生命值）
- 多关卡 JSON 配置加载
- 道具系统（加长挡板、多球、减速）
- 排行榜持久化
- 简易局域网联机（主机/客户端）
- 关卡编辑与保存

> 仓库根目录同时包含 raylib 源码与若干示例目录；若你只想运行游戏，请进入 `Breakout/` 进行构建。

## 编译运行步骤

### 1) 环境准备（Linux/WSL）
- CMake >= 3.10
- 支持 C++17 的编译器（g++/clang++）
- X11/OpenGL 相关运行环境

### 2) 编译 raylib（在仓库根目录）
```bash
cmake -S . -B build -DPLATFORM=Desktop
cmake --build build -j
```

### 3) 编译游戏（进入 Breakout 目录）
```bash
cd Breakout
cmake -S . -B build
cmake --build build -j
```

### 4) 运行
```bash
cd build
./breakout_week2
```

> 首次构建后会自动复制 `levels/` 与 `config.json` 到可执行文件目录。

## 操作说明

### 基本控制
- `← / A`：挡板左移
- `→ / D`：挡板右移
- `Space`：发射小球 / 在菜单中确认
- `P`：暂停/继续
- `Esc`：返回或退出（视界面状态而定）

### 菜单与模式
- 主菜单可进入：开始游戏、排行榜、联机相关选项（若启用）
- 编辑模式支持选择砖块类型并保存布局

### 联机提示
- 主机（Host）负责广播状态
- 客户端（Client）接收状态并同步关键输入
- UI 中会显示当前联机状态提示信息

## 功能列表
- [x] 小球物理移动与碰撞反弹
- [x] 砖块命中判定与销毁
- [x] 关卡加载（`levels/level*.json`）
- [x] 道具掉落与效果生命周期管理
- [x] 粒子效果
- [x] 分数/生命值/胜利判定
- [x] 本地存档（继续游戏）
- [x] 排行榜持久化
- [x] 简易联机状态同步
- [x] 关卡编辑与导出
