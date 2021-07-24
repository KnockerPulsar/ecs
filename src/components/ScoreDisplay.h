#include "Component.h"
#include "../../vendor/raylib-cpp/raylib-cpp.hpp"
#include "../Entity.h"
#include "Paddle.h"
#include "../TUtils.h"

namespace pong
{
    class ScoreDisplay : public Component
    {
    private:
        raylib::Vector2 displayPos;
        raylib::Color color;
        raylib::Window *win;
        int fSize;

    public:
        ScoreDisplay(int fontSize, raylib::Window *w)
        {
            fSize = fontSize;
            tag = tags::indep;
            win = w;
        }
        ~ScoreDisplay() {}

        void Start() override
        {
            Paddle *pad = TUtils::GetComponentFromEntity<Paddle>(entityID);

            displayPos.y = win->GetHeight() * 0.2f;
            color = pad->BoxColor;

            // Left player
            if (pad->playerNum == 1)
            {
                displayPos.x = win->GetWidth() / 4;
            }
            // Right player
            else
            {
                displayPos.x = win->GetWidth() / 4 * 3;
            }
        }
        void Update() override
        {
            // AAAAAAAAAAAAA
            // Have to do this instead of using pointers
            // Since pointers might introduce issues if we want to save and
            // load from disk
            Entity *entt = Entity::GetEntity(entityID);
            Paddle *pad = dynamic_cast<Paddle *>(entt->GetComponent(typeid(Paddle)));
            DrawText(std::to_string(pad->score).c_str(), displayPos.x, displayPos.y, fSize, color);
        }
    };
}