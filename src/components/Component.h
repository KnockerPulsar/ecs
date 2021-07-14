#pragma once

// Abstract class to derive all components from
namespace pong
{
  class Component
  {
  public:
    Component() {}
    virtual ~Component(){};
    virtual void Update(std::vector<pong::Component *> *data) = 0;
    virtual void Start(){};
    virtual bool CheckCollision(Component *other) {return false;};
    virtual void OnCollision(Component* other) {}
  };
}
