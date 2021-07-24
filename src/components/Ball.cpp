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

// Forward Declarations
void XCHG(float &A, float &B);
bool Between(float less, float value, float more);

namespace pong
{
    Ball::Ball(raylib::Texture2D *goalPart, raylib::Vector2 *position, float radius, float minSpeed, float maxSpeed)
    {
        tag = tags::indep;
        this->radius = radius;
        this->minSpeed = minSpeed;
        this->maxSpeed = maxSpeed;
        this->position = position;
        initialPos = *position;
        goalParticle = goalPart;
    }
    Ball::~Ball() {}

    void Ball::Start()
    {
        *position = initialPos;
        // Generates an angle between -45 and 45 OR 135 and 225
        GenerateRandomVelocity(-PI / 4, PI / 4, minSpeed, maxSpeed);
        float speed = velocity.x * velocity.x + velocity.y * velocity.y;
        BallCollision* bColl = TUtils::GetComponentFromEntity<BallCollision>(entityID);
        bColl->collidingWith.clear();
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
        if (position->x < -DeleteThreshold ||
            position->y < -DeleteThreshold ||
            position->x > GetScreenWidth() + DeleteThreshold ||
            position->y > GetScreenHeight() + DeleteThreshold)
        {
            TraceLog(LOG_DEBUG, std::to_string(1 / GetFrameTime()).c_str());
        }
    }

    void Ball::Update()
    {
        Accelerate();
        // UpdateCollision();
        // CheckCollisions(objs);
        DrawCircle(position->x, position->y, radius, RAYWHITE);

        Move();
    }

    void Ball::Accelerate()
    {
        float speedScalar = 1 + 0.001 * std::min(GetFrameTime(), (float)(1.0 / 60));
        velocity *= speedScalar;
    }

    // Let's say we collided with one player...
    // We need to get info about that entity
    // Given a pointer to a component, we need a reference to an entity
    // Let's store it in the base component
    void Ball::OnCollisionEnter(Component *other)
    {
        Paddle *colliderPaddle = TUtils::GetComponentFromEntity<Paddle>(other->entityID);
        Wall *collWall = TUtils::GetComponentFromEntity<Wall>(other->entityID);
        Net *collNet = TUtils::GetComponentFromEntity<Net>(other->entityID);
        if (colliderPaddle)
        {
            std::string whichPlayer;
            if (colliderPaddle)
            {
                whichPlayer = colliderPaddle->playerNum == 1 ? "left" : "right";
                TraceLog(LOG_DEBUG, ("Collided with the " + whichPlayer + " player").c_str());

                float velMag = velocity.Length();
                float paddleMiddleY =
                    colliderPaddle->position->y + colliderPaddle->size.y / 2;
                float segmentSize = colliderPaddle->size.y / colliderPaddle->NUM_SEGMENTS;

                float dy = paddleMiddleY - position->y;
                if (dy == 0)
                {
                    velocity.x = velMag;
                    velocity.y = 0;
                    return;
                }
                // For 6 segments
                //         // | 60 degrees
                //         // | 30 degrees
                //         // | 0 degrees
                //         // | 0 degrees
                //         // | 30 degrees
                //         // | 60 degrees

                for (int i = 0; i < colliderPaddle->NUM_SEGMENTS / 2; i++)
                {
                    float limit = (i + 1) * segmentSize;
                    if (Between(-limit, dy, limit))
                    {
                        // Should be +ve if the ball is above the paddle's middle
                        // -ve if below
                        int aboveOrBelow = dy / abs(dy);
                        int angle = colliderPaddle->playerNum == 1 ? 0 : 180;
                        angle += (i * 30 * -aboveOrBelow * (colliderPaddle->playerNum == 1 ? 1 : -1));

                        velocity.x = std::cos(DEG2RAD * angle) * velMag;
                        velocity.y = std::sin(DEG2RAD * angle) * velMag;
                        break;
                    }
                }
            }
        }
        else if (collWall)
        {
            velocity.y *= -1;
            TraceLog(LOG_DEBUG, "Collided with a wall!");
        }
        else if (collNet)
        {
            Paddle *pad = TUtils::GetComponentFromEntity<Paddle>(collNet->pID);
            pad->score++;

            pong::Entity *particles = new pong::Entity();
            pong::Particle **partComps = new pong::Particle *[100];

            for (int i = 0; i < 100; i++)
            {
                partComps[i] = new Particle(goalParticle, Utils::GetRand(0.01, 0.1));
                partComps[i]->position = *position;
                partComps[i]->delay = i * 0.0025;
                raylib::Vector2 rand(Utils::GetRand(-50, 50), Utils::GetRand(-50, 50));
                partComps[i]->vel = rand - raylib::Vector2(velocity.x, 0) * 3;
                partComps[i]->rotation = Utils::GetRand(0, 360);
                partComps[i]->tint = pad->BoxColor;
                particles->AddComponent(partComps[i]);
            }

            Start();
        }
    }
} // namespace pong
