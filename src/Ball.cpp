#include "src/Ball.h"
#include "math.h"
#include "src/ExtraFunctions.cpp"
#include "src/Random.cpp"

// Forward Declarations
void XCHG(float &A, float &B);
bool Between(float less, float value, float more);

float COLL_CHECK_THREASHOLD;

namespace pong
{
    Ball::Ball(Vector2 *position, float radius, float minSpeed, float maxSpeed)
    {
        tag = tags::indep;
        this->radius = radius;
        this->minSpeed = minSpeed;
        this->maxSpeed = maxSpeed;
        this->position = position;
    }
    Ball::~Ball() {}

    void Ball::Start()
    {
        GenerateRandomVelocity(0, 2 * PI, minSpeed, maxSpeed);
        float speed = velocity.x * velocity.x + velocity.y * velocity.y;
        COLL_CHECK_THRESHOLD = speed * speed;
    }

    // angleMin and angleMax are in RADIANS
    void Ball::GenerateRandomVelocity(float angleMin, float angleMax, float magMin, float magMax)
    {
        float angle = Random::GetRand(angleMin, angleMax);
        float magnitude = Random::GetRand(magMin, magMax);

        velocity.x = magnitude * cos(angle);
        velocity.y = magnitude * sin(angle);
    }

    void Ball::Move()
    {
        if (!canMove)
            return;

        float deltaTime = GetFrameTime();
        position->x += velocity.x * deltaTime;
        position->y += velocity.y * deltaTime;

        if (position->x < 0 || position->x > GetScreenWidth())
            position->x = GetScreenWidth() / 2;

        if (position->y < 0 || position->y > GetScreenHeight())
            position->y = GetScreenHeight() / 2;
        
    }

    void Ball::Update(std::vector<pong::Component *> *data)
    {
        Accelerate();
        // UpdateCollision();
        // CheckCollisions(objs);
        DrawCircle(position->x, position->y, radius, RAYWHITE);

        Move();
    }

    void Ball::Accelerate()
    {
        float speedScalar = 1 + 0.001 * std::min(GetFrameTime(), MAX_FRAMETIME);
        velocity = Vector2Multiply(velocity, speedScalar);
    }

    // Could be cleaned up with a virtual method in each object child
    // void OnCollision(Object *other, Rectangle &otherRect)
    // {
    //     // This would restrict the passed data though,
    //     // Unless we'd create specific structs to pass data
    //     // (which isn't a bad idea, I'm just lazy)
    //     // Collect data
    //     // other->OnCollision(data);

    //     if (Wall *otherWall = dynamic_cast<Wall *>(other))
    //     {
    //         if (!otherWall->player)
    //         {
    //             velocity.y *= -1;
    //         }
    //         else
    //         {
    //             otherWall->player->score++;
    //             Start();
    //         }
    //     }
    //     else if (Paddle *pad = dynamic_cast<Paddle *>(other))
    //     {
    //         // Check where the ball is relative to the paddle
    //         // The center segments return the ball a 90° angle in relation to the paddle, while the outer segments return the ball at smaller angles.
    //         // The angle here is between the paddle surface and the ball's return velocity (90 degress = bounce in the opposite direction)
    //         // | 30 degrees
    //         // | 60 degrees
    //         // | 90 degrees
    //         // | 90 degrees
    //         // | 60 degrees
    //         // | 30 degrees

    //         pad->OnCollision(this);

    //         float paddleY = pad->position->y;
    //         paddleY += pad->size.y / 2; // To move the paddleY to be in the center of the paddle

    //         float diff = (paddleY - position->y);

    //         int diffSign = diff / abs(diff);
    //         float segSize = pad->segSize;

    //         float magnitude = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    //         float angle = 0;
    //         // If the ball hit either the center 2 segments, invert the x velocity
    //         if (abs(diff) < segSize)
    //         {
    //             int dir = velocity.x / abs(velocity.x);
    //             velocity.x = magnitude * -dir;
    //             velocity.y = 0;
    //             return;
    //         }
    //         else if (Between(segSize, abs(diff), 2 * segSize))
    //         {
    //             // Bounce with angle 60
    //             angle = (60 + ((velocity.x > 0) ? 90 : 0)) * diffSign;
    //         }
    //         else if (Between(2 * segSize, abs(diff), 3 * segSize))
    //         {
    //             // Bounce with angle 30
    //             angle = (30 + ((velocity.x > 0) ? 90 : 0)) * diffSign;
    //         }
    //         angle *= DEG2RAD;
    //         velocity.x = magnitude * cos(angle);
    //         velocity.y = magnitude * -sin(angle);

    //     }
    // }

    // Checks if the ball wasn't colliding last frame but is colliding now on the y coordinate
    // AKA the player hit the ball with the paddle's side
    // bool CheckIfSideSave(Rectangle &paddleRect, float paddleLastFrameSpeed)
    // {
    //     return !CheckCollisionCircleRec({position->x, position->y - paddleLastFrameSpeed}, radius, paddleRect);
    // }

} // namespace pong
