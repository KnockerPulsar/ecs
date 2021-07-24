#pragma once
#include "../vendor/raylib-cpp/raylib-cpp.hpp"
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

    // A map of all the game's entities, accessed by the entity's ID
    // Definitely not thread safe
    static std::unordered_map<int, Entity *> *entities;

    Entity();
    virtual ~Entity();

    std::vector<Component *> *GetComponents(const std::type_index &type);

    Component *GetComponent(const std::type_index &type);

    Component *GetComponent(const int &id);

    static Entity *GetEntity(const int &id);

    void AddComponent(pong::Component *component);
    void RemoveComponent(int &compID);

    pong::System *AddNewSystem(pong::Component *component);

    virtual void OnCollisionEnter(BaseCollision *caller, Component *other);

    virtual void OnCollisionExit(BaseCollision *caller, Component *other);

    static void PrintEntity();

  };
}
