#pragma once
#include "src/Entity.h"
#include <unordered_map>
#include "src/Component.h"
#include "src/Ball.h"
#include "src/Utils.h"
#include <typeindex>
#include <vector>
#include "src/System.h"

namespace pong
{

    Entity::Entity()
    {
        entityID = Utils::GetUniqueID();

        if (!Entity::entities)
            Entity::entities = new std::unordered_map<int, Entity *>();

        (*Entity::entities)[entityID] = this;
    }
    Entity::~Entity() {}

    std::vector<Component *> *Entity::GetComponents(const std::type_index &type)
    {
        typedef std::unordered_multimap<std::type_index, Component *>::iterator it;
        std::vector<Component *> *comps;

        std::pair<it, it> range = typeComponents.equal_range(type);

        // TODO: Make sure that iter.first == iter.second when there are no components with the specified tag
        if (range.first != range.second)
        {
            comps = new std::vector<Component *>();
            for (it iter = range.first; iter != range.second; ++iter)
            {
                comps->push_back(iter->second);
            }
        }
        return comps;
    }

    Component *Entity::GetComponent(const std::type_index &type)
    {
        return typeComponents.find(type)->second;
    }

    Component *Entity::GetComponent(const int &id)
    {
        return idComponents[id];
    }

    Entity *Entity::GetEntity(const int &id)
    {
        return (*Entity::entities)[id];
    }

    void Entity::AddComponent(std::unordered_map<tags, pong::System *, pong::TagsHashClass> *systems, pong::Component *component)
    {
        if (!component)
            return;

        const tags t = component->tag;
        pong::System *system = (*systems)[t];

        if (!system)
            system = AddNewSystem(systems, component);

        system->AddComponent(component);

        // Set the current entity as its owner
        component->entityID = entityID;

        // Insert it into the type list
        typeComponents.insert({typeid(*component), component});

        // Insert it into the id list
        idComponents[component->componentID] = component;
    }

    pong::System *Entity::AddNewSystem(std::unordered_map<tags, pong::System *, pong::TagsHashClass> *systems, pong::Component *component)
    {
        pong::System *newSys;

        // TODO: Use enable_if and SFINAE to clean up this mess
        newSys = new System(component->tag);

        (*systems)[component->tag] = newSys;
        return newSys;
    }
}
