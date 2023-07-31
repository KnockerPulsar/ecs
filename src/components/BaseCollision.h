#pragma once
#include <unordered_map>
#include <vector>

#include "raylib.h"
#include "raylib-cpp.hpp"
#include "Component.h"

namespace pong {
class RectCollision;
class BallCollision;
class QuadTree;

// Responsible for maintaining the ball's collision
// and detecting its collision with other objects
class BaseCollision : public Component {
public:

  // Should be initialized in Start() to the enitity's position
  Vector2 *position; 

  // A list of other collisions this collision is colliding
  std::unordered_map<int, Component *> collidingWith; 
                    
  // A list of all the nodes this collision is in
  std::vector<QuadTree *>
      currentNodes{}; 

  BaseCollision();

  virtual ~BaseCollision();
  virtual void Update() = 0;

  // Should be called if you implement your own start function
  virtual void Start() {
    position = &GetEntity()->position;
    tag      = tags::coll;
  }

  // Default color is transparent green
  virtual void DrawDebug(raylib::Color col = raylib::Color(226, 28, 255, 240));

  // Given another base collision, should upcast it into it's proper collision
  // Returns true if a collision occured, false if not.
  // Should have a function for all collider types
  // https://stackoverflow.com/questions/22899363/advice-on-class-structure-in-a-collision-detection-system
  // Probably not the most performant method, but I guess it's good enough
  virtual bool CheckCollision(Component *other)     = 0;
  virtual bool CheckCollision(BaseCollision *other) = 0;
  virtual bool CheckCollision(BallCollision *other) = 0;
  virtual bool CheckCollision(RectCollision *other) = 0;

  // Should be Invoked on both collider and colidee
  virtual void OnCollision(Component *other);

  // Called if the 2 colliders are not colliding, removes other from
  // collidingWith
  virtual void ClearCollision(Component *other);

  // Checks if this collider is not on screen, useful for edge cases
  virtual bool IsNotOnScreen() = 0;

  void UpdatePosition(Vector2 *pos) { position = pos; }
};
} // namespace pong
