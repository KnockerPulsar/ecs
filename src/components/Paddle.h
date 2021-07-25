#pragma once
#include "../TUtils.h"
#include "Component.h"
#include "raylib.h"
#include <vector>

// A paddle component is responsible for:
//      The visual representation of the paddle
//      Controlling the paddle's movement
namespace pong
{
    class Ball;

    class Paddle : public Component
    {
    public:
        int score = 0;
        float speed;
        float segSize;
        short playerNum; // Either 1 for WASD control, or 2 for arrow control
        static const int NUM_SEGMENTS = 6;

        Color BoxColor;
        Vector2 size;
        Vector2 *position;

    public:
        Paddle(Vector2 *position, Vector2 size, float speed, short playerNum, Color padColor)
        {
            tag = tags::indep;
            debug_type = "paddle component";

            this->size = size;
            this->speed = speed;
            this->playerNum = playerNum;
            this->position = position;

            BoxColor = padColor;
            segSize = size.y / NUM_SEGMENTS; // 3 pairs of segments
        }
        ~Paddle() override {}

        void Update() override
        {
            CheckInput();
            Draw();
            // FillSegment(0);
        }

        void Draw()
        {
            DrawRectangleV({position->x, position->y}, size, RAYWHITE);

            // Draw Collision segments
            // TODO: Move this to the paddle collision
            for (size_t i = 0; i < NUM_SEGMENTS; i++)
                DrawRectangleLines(position->x, position->y + segSize * i, size.x, segSize, BoxColor);
        }

        void CheckInput()
        {
            float deltaTime = GetFrameTime();

            // If p1 pressed W or p2 pressed Up
            if ((playerNum == 1 && IsKeyDown(KEY_W)) || (playerNum == 2 && IsKeyDown(KEY_UP)))
            {
                // If they hit the top edge, no going up
                float newY = position->y - deltaTime * speed;
                if (newY <= 0)
                    position->y = 0;
                // Otherwise, move up
                else
                {
                    position->y -= deltaTime * speed;
                }
            }

            // Similar to the previous block, but checks if the the bottom edge of the paddle touched the bottom edge
            if ((playerNum == 1 && IsKeyDown(KEY_S)) || (playerNum == 2 && IsKeyDown(KEY_DOWN)))
            {
                float newY = position->y + deltaTime * speed;
                if (newY >= GetScreenHeight() - size.y)
                    position->y = GetScreenHeight() - size.y;
                else
                {
                    position->y += deltaTime * speed;
                }
            }
        }

        // TODO: Move thiss to its own component
        // void ScoreDisplay()
        // {
        //     if (playerNum == 1)
        //         DrawText(&std::to_string(score)[0], GetScreenWidth() / 4, GetScreenHeight() / 8, 80, RED);
        //     else
        //         DrawText(&std::to_string(score)[0], GetScreenWidth() * 3 / 4, GetScreenHeight() / 8, 80, BLUE);
        // }

        void OnCollisionEnter(Component *other) override
        {
            if (TUtils::GetTypePtr<Ball>(other) != nullptr)
            {
                std::string info = "Ball hit ";
                info += playerNum == 1 ? "left" : "right";
                info += " player";
                // TraceLog(LOG_DEBUG, info.c_str());
            }
        }

        void OnCollisionExit(Component *other) override
        {
            // TraceLog(LOG_INFO, "Collision ended!");
        }

        // Vector2 hitSegPos;
        // float dur = -1;
        // void FillSegment(float duration)
        // {
        //     dur = duration > 0 ? duration : dur;
        //     dur -= GetFrameTime();
        //     if (dur < 0)
        //         return;

        //     DrawRectangleV(hitSegPos, {size.x, segSize}, BoxColor);
        // }
    };

} // namespace pong
