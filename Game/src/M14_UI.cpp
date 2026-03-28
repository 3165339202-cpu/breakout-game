#include "M14_UI.h"
#include "raylib.h"
#include <cstdio>

static Font chineseFont;
static const char* UI_TEXT = "Shadow Echo:影子回音2026 - 基础版本";

void InitUI() {
    int codepointCount = 0;
    int *codepoints = LoadCodepoints(UI_TEXT, &codepointCount);
    
    printf("提取到 %d 个唯一字符\n", codepointCount);

    // 加载更大的字体尺寸（64 代替 32）
    chineseFont = LoadFontEx("../fonts/NotoSansSC.otf", 64, codepoints, codepointCount);
    
    if (chineseFont.texture.id == 0) {
        printf("警告：字体加载失败，使用默认字体\n");
        chineseFont = GetFontDefault();
    } else {
        printf("字体加载成功！包含 %d 个字符\n", chineseFont.glyphCount);
    }
    
    UnloadCodepoints(codepoints);
}

void DrawUI() {
    // 字体大小：40（原来的两倍）
    float fontSize = 40.0f;
    float spacing = 2.0f;
    
    // 测量文字宽度
    Vector2 textSize = MeasureTextEx(chineseFont, UI_TEXT, fontSize, spacing);
    
    // 获取屏幕尺寸
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // 计算底部居中位置
    Vector2 textPos = {
        (screenWidth - textSize.x) / 2,      // 水平居中
        screenHeight - textSize.y - 20       // 底部，留 20 像素边距
    };
    
    // 绘制文字
    DrawTextEx(chineseFont, UI_TEXT, textPos, fontSize, spacing, WHITE);
}

void HandleMenuInput() {}


// #include "M14_UI.h"
// #include "raylib.h"
// #include <cstdio>

// static Font chineseFont;
// static const char* UI_TEXT = "Shadow Echo:影子回音201 - 基础版本";

// void InitUI() {
//     int codepointCount = 0;
//     int *codepoints = LoadCodepoints(UI_TEXT, &codepointCount);
    
//     printf("提取到 %d 个唯一字符\n", codepointCount);

//     // 从 build 目录出发的相对路径
//     chineseFont = LoadFontEx("../fonts/NotoSansSC.otf", 32, codepoints, codepointCount);
    
//     if (chineseFont.texture.id == 0) {
//         printf("警告：字体加载失败，使用默认字体\n");
//         chineseFont = GetFontDefault();
//     } else {
//         printf("字体加载成功！包含 %d 个字符\n", chineseFont.glyphCount);
//     }
    
//     UnloadCodepoints(codepoints);
// }

// void DrawUI() {
//     DrawTextEx(chineseFont, UI_TEXT, (Vector2){10, 10}, 20, 1, WHITE);
// }

// void HandleMenuInput() {}
