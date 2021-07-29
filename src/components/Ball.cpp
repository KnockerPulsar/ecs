#include "Ball.h"
#include "math.h"
#include "../Utils.h"
#include "../Entity.h"
#include "Paddle.h"
#include "Wall.h"
#include "Net.h"
#include "Particle.h"
#include <future>
#include "../TUtils.h"
#include "../Event.h"

// Forward Declarations
void XCHG(float &A, float &B);
bool Between(float less, float value, float more);

namespace pong
{

    static void lambda()
    {
        TraceLog(LOG_DEBUG, "Events test");
    }

    Ball::Ball(raylib::Texture2D *goalPart, float radius, float minSpeed, float maxSpeed, float accelRate)
    {
        tag = tags::indep;
        this->radius = radius;
        this->minSpeed = minSpeed;
        this->maxSpeed = maxSpeed;
        this->position = position;
        this->accelRate = accelRate;
        goalParticle = goalPart;
    }

    // Empty destructor since no dynamic allocations
    Ball::~Ball() {}

    void Ball::Start()
    {
        // Should already be registered to an entity by this points
        position = &Entity::GetEntity(entityID)->position;
        initialPos = *position;
        Reset();
    }

    void Ball::Reset()
    {
        *position = initialPos;

        // Generates an angle between -45 and 45 OR 135 and 225
        GenerateRandomVelocity(-PI / 4, PI / 4, minSpeed, maxSpeed);
    }

    // angleMin and angleMax are in RADIANS
    void Ball::GenerateRandomVelocity(float angleMin, float angleMax, float magMin, float magMax)
    {
        float angle = Utils::GetRand(angleMin, angleMax);
        angle += Utils::GetRand(0, 1) > 0.5 ? 0 : PI;

        float magnitude = Utils::GetRand(magMin, magMax);

        velocity.x = magnitude * cos(angle);
        velocity.y = magnitude * sin(angle);
    }

    void Ball::Move()
    {
        if (!canMove)
            return;

        float deltaTime = GetFrameTime();
        *position += velocity * std::min(deltaTime, (float)1.0 / 60);
    }

    void Ball::Update()
    {
        Accelerate();
        DrawCircle(position->x, position->y, radius, RAYWHITE);

        Move();
    }

    void Ball::Accelerate()
    {
        float speedScalar = 1 + accelRate * std::min(GetFrameTime(), (float)(1.0 / 60));
        velocity *= speedScalar;
    }

    void Ball::OnCollisionEnter(Component *other)
    {
        Paddle *colliderPaddle;
        Wall *collWall;
        Net *collNet;

        if (colliderPaddle = TUtils::GetComponentFromEntity<Paddle>(other->entityID))
            PaddleCollision(colliderPaddle);
        else if (collWall = TUtils::GetComponentFromEntity<Wall>(other->entityID))
            velocity.y *= -1;
        else if (collNet = TUtils::GetComponentFromEntity<Net>(other->entityID))
            NetCollision(collNet);
    }
    void Ball::PaddleCollision(Paddle *colliderPaddle)
    {
        float velMag = velocity.Length();
        float paddleMiddleY =
            colliderPaddle->position->y + colliderPaddle->size.y / 2;
        float segmentSize = colliderPaddle->size.y / colliderPaddle->NUM_SEGMENTS;

        float dy = paddleMiddleY - position->y;
        if (dy == 0)
        {
            velocity.x = velMag;
            velocity.y = 0;

            // Create an entity that constantly draws a rectangle over the hit segment
            Event::AddEvent(0, [] {

            });
            return;
        }

        // TODO: Add a bit of randomness
        // For 6 segments
        //         // | 60 degrees
        //         // | 30 degrees
        //         // | 0 degrees
        //         // | 0 degrees
        //         // | 30 degrees
        //         // | 60 degrees

        // Checking against all the paddles segments to figure out the bouncing angle
        for (int i = 0; i < colliderPaddle->NUM_SEGMENTS / 2; i++)
        {
            float limit = (i + 1) * segmentSize;
            if (Utils::Between(-limit, dy, limit))
            {
                // Should be +ve if the ball is above the paddle's middle
                // -ve if below
                int aboveOrBelow = dy / abs(dy);
                int angle = colliderPaddle->playerNum == 1 ? 0 : 180;

                // I honestly have no idea how I figured this out
                angle += (i * 30 * -aboveOrBelow * (colliderPaddle->playerNum == 1 ? 1 : -1));

                // Adds some randomness to the bounce
                // The randomness scales inversely with how far the ball is from the center
                // ±5 pixels = perfect bounce back. Otherwise, scale randomnesss
                angle += Utils::GetRand(-5, 5) * abs(Utils::BetweenEq(-3, dy, 3) ? 0 : 10 / dy);

                velocity.x = std::cos(DEG2RAD * angle) * velMag;
                velocity.y = std::sin(DEG2RAD * angle) * velMag;
                break;
            }
        }
    }

    void Ball::NetCollision(Net *collNet)
    {
        Paddle *pad = TUtils::GetComponentFromEntity<Paddle>(collNet->pID);
        pad->score++;

        // std::function<void()> fn = lambda;
        // TODO: I think this leaks memory
        std::vector<Particle *> partComps(100);
        for (int i = 0; i < 100; i++)
        {
            partComps[i] = new Particle(goalParticle, Utils::GetRand(0.01, 0.1));
            partComps[i]->position = *position;
            partComps[i]->delay = i * 0.0025;
            raylib::Vector2 rand(Utils::GetRand(-50, 50), Utils::GetRand(-50, 50));
            partComps[i]->vel = rand - raylib::Vector2(velocity.x, 0) * 3;
            partComps[i]->rotation = Utils::GetRand(0, 360);
            partComps[i]->tint = pad->BoxColor;
            partComps[i]->scale = 0.01;
            partComps[i]->lifetime = Utils::GetRand(0.1, 3);
            System::AddComponentIndep(partComps[i]);
        }

        Reset();
    }

} // namespace pong
