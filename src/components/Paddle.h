#pragma once
#include "../TUtils.h"
#include "Component.h"
#include "raylib.h"
#include <vector>
#include "Wall.h"
#include "../Event.h"
#include <algorithm>
#include "../Game.h"

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
        float speed,
            segSizeY; // The size of the paddle's segments in the Y axis
                      // Note that all segments share the same size

        short playerNum; // Either 1 for WASD control, or 2 for arrow control
        static const int NUM_SEGMENTS = 6;

        Color BoxColor;
        Vector2 size, *position;

        bool fillSeg = false;
        int segToFill;

    public:
        Paddle(Vector2 size, float speed, short playerNum, Color padColor)
        {
            tag = tags::indep;
            debug_type = "paddle component";

            this->size = size;
            this->speed = speed;
            this->playerNum = playerNum;

            BoxColor = padColor;
            segSizeY = size.y / NUM_SEGMENTS; // 3 pairs of segments
        }

        void Update() override
        {
            CheckInput();
            Draw();
            FillSegment();
        }

        void Start() override { this->position = &GetEntity()->position; }

        void Reset() override
        {
            score = 0;
        }

        void Draw()
        {
            DrawRectangleV({position->x, position->y}, size, RAYWHITE);

            // Draw Collision segments
            for (size_t i = 0; i < NUM_SEGMENTS; i++)
                DrawRectangleLines(position->x, position->y + segSizeY * i, size.x, segSizeY, BoxColor);
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
                    position->y -= deltaTime * speed;
            }

            // Similar to the previous block, but checks if the the bottom edge of the paddle touched the bottom edge
            if ((playerNum == 1 && IsKeyDown(KEY_S)) || (playerNum == 2 && IsKeyDown(KEY_DOWN)))
            {
                float newY = position->y + deltaTime * speed;
                int screenHeight = GetScreenHeight();

                if (newY >= screenHeight - size.y)
                    position->y = screenHeight - size.y;
                else
                    position->y += deltaTime * speed;
            }
        }

        void FillSegment()
        {
            if (!fillSeg)
                return;
            else
            {
                int yPos = position->y + segToFill * segSizeY;
                DrawRectangle(position->x, yPos, size.x, segSizeY, BoxColor);
            }
        }

        void OnCollisionEnter(Component *other) override
        {
            Wall *wall;
            Ball *ball;
            if (wall = TUtils::GetComponentFromEntity<Wall>(other->entityID))
            {
                TraceLog(LOG_DEBUG, "Entered collision with a wall");
            }
            else if (ball = TUtils::GetComponentByType<Ball>(other->entityID))
            {
                // Fill the hit segment for 0.5 seconds
                fillSeg = true;

                // Figure out which segment to fill
                // Segments are ordered from 0 - NUM_SEGMENTS-1 from top to bottom
                // Note that the paddle's position starts at the upper left corner
                float dy = other->GetEntity()->position.y - position->y;

                // If dy < segsize , dy/segSize = 0
                //    dy > segSize , dy/segSize = 1
                // And so on (integer division)
                segToFill = std::min((int)dy / (int)segSizeY, NUM_SEGMENTS - 1);

               Game::currScene->AddEvent(0.5, [this]
                                { this->fillSeg = false; });
            }
        }

        // void OnCollisionStay(Component *other) override
        // {
        //     Wall *wall;
        //     if (wall = TUtils::GetComponentFromEntity<Wall>(other->entityID))
        //     {
        //         TraceLog(LOG_DEBUG, "Staying in collision with a wall");
        //     }
        // }
        // void OnCollisionExit(Component *other) override
        // {

        //     Wall *wall;
        //     if (wall = TUtils::GetComponentFromEntity<Wall>(other->entityID))
        //     {
        //         TraceLog(LOG_DEBUG, "Exited collision with a wall");
        //     }
        // }
    };

} // namespace pong
