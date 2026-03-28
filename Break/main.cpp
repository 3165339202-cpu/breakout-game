#include "raylib.h"
#include <vector>

#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
int main()
{
    InitWindow(800, 600, "Breakout");

    Ball ball;
    ball.Init(400, 300);

    Paddle paddle;
    paddle.Init(350, 550);

    std::vector<Brick> bricks;

    for(int i=0;i<5;i++)
        for(int j=0;j<8;j++)
        {
            Brick b;
            b.Init(80 + j*80, 50 + i*30);
            bricks.push_back(b);
        }

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // 更新
        ball.Update();
        paddle.Update();

        // 边界碰撞
        if(ball.position.x < 0 || ball.position.x > 800)
            ball.speed.x *= -1;
        
        if(ball.position.y > 600)
        {
            DrawText("GAME OVER", 300, 300, 40, RED);
        }
        if(ball.position.y < 0)
            ball.speed.y *= -1;

        // 球撞板
        if(CheckCollisionCircleRec(ball.position, ball.radius, paddle.rect))
            ball.speed.y *= -1;

        // 球撞砖块
        for(auto &b : bricks)
        {
            if(b.active && CheckCollisionCircleRec(ball.position, ball.radius, b.rect))
            {
                b.active = false;
                ball.speed.y *= -1;
            }
        }

        // 绘制
        BeginDrawing();
        ClearBackground(BLACK);

        ball.Draw();
        paddle.Draw();

        for(auto &b : bricks)
            b.Draw();

        EndDrawing();
    }

    CloseWindow();
}