#include "Entity.h"
#include <vector>
#include <typeindex>
#include <unordered_map>
#include "components/Component.h"
#include "components/Ball.h"
#include "Utils.h"
#include "System.h"

namespace pong
{

    Entity::Entity(float x, float y)
    {
        entityID = Utils::GetUniqueID();
        initPos = raylib::Vector2(x,y);
        position = initPos;

        Game::currScene->entites[entityID] = this;
    }
    
    Entity::~Entity() { TraceLog(LOG_DEBUG, "Entity destroyed"); }

    void Entity::Reset()
    {
        position = initPos;
        for (auto &&comp : idComponents)
        {
            comp.second->Reset();
        }
        
    }

    std::vector<Component *> *Entity::GetComponents(const std::type_index &type)
    {
        typedef std::unordered_multimap<std::type_index, Component *>::iterator it;
        std::vector<Component *> *comps;

        std::pair<it, it> range = typeComponents.equal_range(type);

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
        auto found = typeComponents.find(type);
        if (found != typeComponents.end())
            return found->second;
        else
            return nullptr;
    }

    Component *Entity::GetComponent(const int &id)
    {
        return idComponents[id];
    }

    Entity *Entity::GetEntity(const int &id)
    {
        return Game::currScene->entites[id];
    }

    Entity& Entity::AddComponent(pong::Component *component)
    {
        auto systems = Game::currScene->systems;

        if (!component)
            return *this;

        const tags t = component->tag;
        pong::System *system = systems[t];

        if (!system)
            system = AddNewSystem(component);

        system->AddComponent(component);

        // Set the current entity as its owner
        component->entityID = entityID;

        // Insert it into the type list
        typeComponents.insert({typeid(*component), component});

        // Insert it into the id list
        idComponents[component->componentID] = component;

        // Return a reference to the current entity to allow for chaining.
        return *this;
    }

    void Entity::RemoveComponent(int &compID)
    {
        // Check if a component with the same ID exists on the entity
        // Return if not
        auto found = idComponents.find(compID);
        if (found == idComponents.end())
            return;

        // The `found` iterator contains 2 things.
        // found.first = compID, found.second = pointer to component
        Component *comp = found->second;
        typeComponents.erase(typeid(*comp));
        idComponents.erase(found);
        Game::currScene->systems[comp->tag]->RemoveComponent(comp);
    }

    pong::System *Entity::AddNewSystem(pong::Component *component)
    {
        pong::System *newSys;

        newSys = new System(component->tag);

        Game::currScene->systems[component->tag] = newSys;
        return newSys;
    }

    void Entity::OnCollisionEnter(BaseCollision *caller, Component *other)
    {
        for (auto &&comp : idComponents)
        {
            if (comp.second != caller)
                comp.second->OnCollisionEnter(other);
        }
    }

    void Entity::OnCollisionStay(BaseCollision *caller, Component *other)
    {
        for (auto &&comp : idComponents)
        {
            if (comp.second != caller)
                comp.second->OnCollisionStay(other);
        }
    }

    void Entity::OnCollisionExit(BaseCollision *caller, Component *other)
    {
        for (auto &&comp : idComponents)
        {
            if (comp.second != caller)
                comp.second->OnCollisionExit(other);
        }
    }

    void Entity::EnableDisableAll(bool enable)
    {
        for (auto &&comp : idComponents)
        {
            comp.second->enabled = enable;
        }
    }

    void Entity::EnableDisableComponent(int &compID, bool enable)
    {
        if (idComponents.find(compID) != idComponents.end())
            idComponents[compID]->enabled = enable;
    }

    void Entity::PrintEntity()
    {

        std::string str;
        for (auto i : Game::currScene->entites)
        {

            str = "Entity: " + std::to_string(i.second->entityID) + ", number of components: " +
                  std::to_string(i.second->idComponents.size()) + '\n';
            for (auto &&comp : i.second->idComponents)
            {
                str += "\t Component: " + std::to_string(comp.second->componentID) + '\n';
            }
        }
        // TraceLog(LOG_DEBUG, str.c_str());
    }
}
