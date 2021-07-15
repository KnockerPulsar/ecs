#include "../Includes.h"
class Wall : public Object
{

public:
    Paddle *player;

    short type; // 0: Horizontal, 1: Vertical
    Wall(Vector3 position, Vector3 size, short type, Paddle *player = nullptr)
    {
        transform.position = position;
        transform.scale = size;
        collision.min = position;
        collision.max = {position.x + size.x, position.y + size.y, 0};
        this->type = type;
        this->player = player;
    }
    ~Wall() {}
    void Update(std::vector<Object *> &objs)
    {
    }
};
