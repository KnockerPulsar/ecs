#include "Component.h"
#include "../../include/raylib-cpp/include/raylib-cpp.hpp"
#include "../Entity.h"
#include "Paddle.h"
#include "../TUtils.h"

namespace pong
{
    class ScoreDisplay : public Component
    {
    private:
        raylib::Vector2 displayPos; // Where to display the score
        raylib::Color color;        // Font color
        int fSize;                  // Font size
        Paddle *pad;                // To cache the paddle pointer on startup

    public:
        ScoreDisplay(int fontSize)
        {
            fSize = fontSize;
            tag = tags::indep;
        }

        ~ScoreDisplay() {}

        void Start() override
        {
            pad = TUtils::GetComponentFromEntity<Paddle>(entityID);

            displayPos.y = GetScreenHeight() * 0.2f;
            color = pad->BoxColor;

            // Left player
            if (pad->playerNum == 1)
            {
                displayPos.x = GetScreenWidth() / 4;
            }
            // Right player
            else
            {
                displayPos.x = GetScreenWidth() / 4 * 3;
            }
        }
        
        void Update() override
        {
            DrawText(std::to_string(pad->score).c_str(), displayPos.x, displayPos.y, fSize, color);
        }
    };
}