#include "Component.h"
namespace pong
{
    class Wall : public Component
    {
    private:
        /* data */
    public:
        Wall(/* args */) { tag = tags::indep; }
        ~Wall() {}
    };
}