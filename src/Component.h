#pragma once
#include <string>
#include "src/Tags.h"

// Abstract class to derive all components from
namespace pong
{
  class BaseCollision;
  class BallCollision;
  class RectCollision;

  class Component
  {
  public:
    tags tag = tags::null;
    int entityID; // Might be better than a pointer if we want to save to disk
    int componentID;
    Component() {}
    virtual ~Component(){};
    virtual void Update(std::vector<pong::Component *> *data) = 0;
    virtual void Start(){};
    virtual bool CheckCollision(Component *other) {}
    virtual bool CheckCollision(BaseCollision *other) {}
    virtual bool CheckCollision(BallCollision *other) {}
    virtual bool CheckCollision(RectCollision *other) {}
    virtual void OnCollision(Component *other) {}
  };
}
