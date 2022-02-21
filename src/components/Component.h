#pragma once
#include <string>
#include <vector>
#include "../Tags.h"
#include "../Utils.h"
#include "../Entity.h"

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

    // Why use IDs?
    // Might be better than a pointer if we want to save to disk
    // I guess caching pointers is alright as long as we re-cache them
    // on loading from desk, depends on how it's implemented
    // If you don't intend on saving to disk, you can just use pointers
    int entityID = -1;
    int componentID;

    std::string debug_type; // For debugging purposes
    bool enabled = true;    // Controls whether the update function executes for this component or not

    Component() { componentID = Utils::GetUniqueID(); }
    virtual ~Component() {}
    virtual void Update() {}
    virtual void Start() {}
    virtual void Reset() {}
    virtual bool CheckCollision(Component *other) { return false; }
    virtual bool CheckCollision(BaseCollision *other) { return false; }
    virtual bool CheckCollision(BallCollision *other) { return false; }
    virtual bool CheckCollision(RectCollision *other) { return false; }
    virtual void OnCollision(Component *other) {}
    virtual void ClearCollision(Component *other) {}
    virtual void OnCollisionEnter(Component *other) {}
    virtual void OnCollisionStay(Component *other) {}
    virtual void OnCollisionExit(Component *other) {}
    virtual Entity *GetEntity() { return Entity::GetEntity(entityID); }
  };
}
