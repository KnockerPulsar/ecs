#pragma once
#include "components/BallCollision.h"
#include "Entity.h"
namespace pong
{
    class Component;
    namespace TUtils
    {
        template <typename T>
        T *GetTypePtr(Component *comp)
        {
            return dynamic_cast<T *>(comp);
        }

        template <typename T>
        T *GetComponentByType(const int &eID)
        {
            auto& typeComponents = Entity::GetEntity(eID)->typeComponents;

            auto found = typeComponents.find(typeid(T));
            if (found != typeComponents.end())
                return dynamic_cast<T *>(found->second);
            else
                return nullptr;
        }

        template <typename T>
        
        static T *GetComponentFromEntity(const int &eID)
        {
            Entity *entt = Entity::GetEntity(eID);
            if (entt)
                return GetComponentByType<T>(eID);
            else
                return nullptr;
        }

    }
}