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

    // Have to initialize it here...
    std::unordered_map<int, Entity *> *Entity::entities = new std::unordered_map<int, Entity *>();

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
        return (*Entity::entities)[id];
    }

    void Entity::AddComponent(pong::Component *component)
    {
        auto systems = System::systems;

        if (!component)
            return;

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
    }

    //  TODO: Check why it seg faults here
    void Entity::RemoveComponent(int &compID)
    {
        auto found = idComponents.find(compID);
        if(found == idComponents.end()) return;

        Component *comp = found->second;
        typeComponents.erase(typeid(*comp));
        idComponents.erase(found);
        System::systems[comp->tag]->RemoveComponent(comp);
    }

    pong::System *Entity::AddNewSystem(pong::Component *component)
    {
        pong::System *newSys;

        // TODO: Use enable_if and SFINAE to clean up this mess
        newSys = new System(component->tag);

        System::systems[component->tag] = newSys;
        return newSys;
    }

    // TODO: Maybe use function pointers to make this less repitive?
    void Entity::OnCollisionEnter(BaseCollision *caller, Component *other)
    {
        for (auto &&comp : idComponents)
        {
            if (comp.second != caller)
                comp.second->OnCollisionEnter(other);
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
    void Entity::PrintEntity()
    {

        std::string str;
        for (auto i : *Entity::entities)
        {

            str = "Entity: " + std::to_string(i.second->entityID) + ", number of components: " +
                  std::to_string(i.second->idComponents.size()) + '\n';
            for (auto &&comp : i.second->idComponents)
            {
                str += "\t Component: " + std::to_string(comp.second->componentID) + '\n';
            }
        }
        TraceLog(LOG_DEBUG, str.c_str());
    }
}
