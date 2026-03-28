#include "M02_Input.h"
#include "raylib.h"

Action GetAction() {
    if (IsKeyPressed(KEY_SPACE)) return Action::SHADOW;
    if (IsKeyDown(KEY_RIGHT)) return Action::RIGHT;
    if (IsKeyDown(KEY_LEFT)) return Action::LEFT;
    if (IsKeyPressed(KEY_UP)) return Action::JUMP;
    if (IsKeyPressed(KEY_P)) return Action::PAUSE;
    return Action::NONE;
}