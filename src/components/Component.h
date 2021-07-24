#pragma once
#include <string>
#include <vector>
#include "../Tags.h"
#include "../Utils.h"

// Abstract class to derive all components from
namespace pong
{
  class BaseCollision;
  class RectCollision;
  class BallCollision;

  class Component
  {
  public:
    tags tag = tags::null;
    int entityID; // Might be better than a pointer if we want to save to disk
    int componentID;
    std::string debug_type;
    Component() { componentID = Utils::GetUniqueID(); }
    virtual ~Component(){};
    virtual void Update(){};
    virtual void Start(){};
    virtual bool CheckCollision(Component *other) { return false; }
    virtual bool CheckCollision(BaseCollision *other) { return false; }
    virtual bool CheckCollision(BallCollision *other) { return false; }
    virtual bool CheckCollision(RectCollision *other) { return false; }
    virtual void OnCollision(Component *other) {}
    virtual void ClearCollision(Component *other) {}
    virtual void OnCollisionEnter(Component *other) {}
    virtual void OnCollisionExit(Component *other) {}

  };
}
