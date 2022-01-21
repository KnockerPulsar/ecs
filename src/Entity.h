#pragma once
#include "../include/raylib-cpp/include/raylib-cpp.hpp"
#include "Tags.h"
#include <unordered_map>
#include <typeindex>

namespace pong
{
  class System;
  class Component;
  class BaseCollision;

  class Entity
  {
  private:
    raylib::Vector2 initPos;
    
  public:
    raylib::Vector2 position;
    int entityID;

    // Stores a multimap of the entity's components by componentID
    // Might not be super useful right now since the only place you can access the component's id is through
    // the component itself
    std::unordered_map<int, Component *> idComponents;

    // Stores a multimap of the entity's components by type
    // To allow for multiple components with the same key
    std::unordered_multimap<std::type_index, pong::Component *> typeComponents;

    // Constructor with 2D coordinates
    Entity(float x, float y);

    ~Entity();

    void Reset();

    std::vector<Component *> *GetComponents(const std::type_index &type);

    Component *GetComponent(const std::type_index &type);

    Component *GetComponent(const int &id);

    static Entity *GetEntity(const int &id);

    void AddComponent(pong::Component *component);

    void RemoveComponent(int &compID);

    pong::System *AddNewSystem(pong::Component *component);

    // These three functions are almost the same, there's probably a cleaner way to do this
    void OnCollisionEnter(BaseCollision *caller, Component *other);
    void OnCollisionStay(BaseCollision *caller, Component *other);
    void OnCollisionExit(BaseCollision *caller, Component *other);

    void EnableDisableAll(bool enable);

    void EnableDisableComponent(int &compID, bool enable);

    static void PrintEntity();
  };
}
