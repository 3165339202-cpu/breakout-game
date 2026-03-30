#ifndef INPUT_H
#define INPUT_H

#include "Common.h"

Action GetAction();     // 每帧调用，返回当前按下的主要动作（优先级：影子>跳跃>左右>无）

#endif