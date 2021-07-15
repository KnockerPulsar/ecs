#pragma once
#include <unordered_map>
#include "src/Component.h"
#include "src/Ball.h"
#include <typeindex>

namespace pong
{

  class Entity
  {
  private:
    // Stores a multimap of the entity's components by type
    // To allow for multiple components with the same key
    std::unordered_multimap<std::type_index, pong::Component *> typeComponents;

    // Stores a multimap of the entity's components by componentID
    // Might not be super useful right now since the only place you can access the component's id is through
    // the component itself
    std::unordered_map<int, Component *> idComponents;

    // A map of all the game's entities, accessed by the entity's ID
    // Definitely not thread safe
    static std::unordered_map<int, Entity *> *entities;

  public:
    Vector2 position;
    int entityID;

    Entity();
    virtual ~Entity();

    std::vector<Component *> *GetComponents(const std::type_index &type);

    Component *GetComponent(const std::type_index &type);

    Component *GetComponent(const int &id);

    static Entity *GetEntity(const int &id);

    void AddComponent(std::unordered_map<tags, pong::System *, pong::TagsHashClass> *systems, pong::Component *component);
    pong::System *AddNewSystem(std::unordered_map<tags, pong::System *, pong::TagsHashClass> *systems, pong::Component *component);
  };

}

// Have to initialize it here...
std::unordered_map<int, pong::Entity *> *pong::Entity::entities = new std::unordered_map<int, Entity *>();
