#pragma once
#include <map>
#include "components/Component.h"

namespace pong
{
  class Entity
  {
  private:
    std::map<std::string, pong::Component *> components;

  public:
    Vector3 position;
    Entity() {}
    virtual ~Entity() {}
    Component *GetComponenet(const std::string &compName)
    {
      return components.at(compName);
    }

    void AddComponent(std::map<std::string, pong::System *> *systems, pong::Component *component)
    {
      if (!component)
        return;

      pong::System *system = (*systems)[typeid(*component).name()];

      if (!system)
        system = AddNewSystem(systems, component);

      system->AddComponent(component);

      components[typeid(component).name()] = component;
    }

    pong::System *AddNewSystem(std::map<std::string, pong::System *> *systems, pong::Component *component)
    {
      pong::System *newSys; 

      // TODO: Use enable_if and SFINAE to clean up this mess
      if (Paddle *paddle = dynamic_cast<Paddle *>(component))
        newSys = new System(paddle);
      else if (Ball *ball = dynamic_cast<Ball *>(component))
        newSys = new System(ball);
      else if (BaseCollision *baseCol = dynamic_cast<BaseCollision *>(component))
        newSys = new System(baseCol);

      (*systems)[typeid(*component).name()] = newSys;
      return newSys;
    }
  };
}
