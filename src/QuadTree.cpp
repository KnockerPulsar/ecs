#include "QuadTree.h"
#include "TUtils.h"
#include <iostream>
#include <vector>

namespace pong {
int QuadTree::currQuads = 0;

std::tuple<float, float, float, float> GetChildCoords(RectCollision rect,
                                                      int childIndex) {
  float xc, yc, x = rect.position->x, y = rect.position->y, w = rect.width,
                h = rect.height;
  switch (childIndex) {

  case 0: {
    xc = x, yc = y;
  } break;

  case 1: {
    xc = x + w / 2;
    yc = y;
  } break;

  case 2: {
    xc = x;
    yc = y + h / 2;
  } break;

  case 3: {
    xc = x + w / 2;
    yc = y + h / 2;
  } break;
  }
  return {xc, yc, w / 2, h / 2};
}

// Every quadtree node should check which collisions in the given vector are
// inside it
QuadTree::QuadTree(RectCollision passedColl, int depth)
    : collision(passedColl) {
  if (depth > MAX_DEPTH)
    return;

  children  = std::vector<QuadTree *>(NUM_CHILDREN, nullptr);
  collision = passedColl;

  for (int i = 0; i < NUM_CHILDREN; i++) {
    auto [x, y, w, h] = GetChildCoords(passedColl, i);
    Vector2 *childPos = new Vector2{x, y};
    children[i]       = new QuadTree(RectCollision(childPos, w, h), depth + 1);
  }
}

void QuadTree::Insert(Component *comp, int depth) {
  if (!collision.CheckCollision(comp))
    return;

  if ((contained.size() < MAX_CONTAINED && !subdivided) || depth == MAX_DEPTH-1) {
    contained.push_back(comp);
    TUtils::GetTypePtr<BaseCollision>(comp)->currentNodes.push_back(this);
    myNum = currQuads;
    used=true;
    currQuads++;
    return;
  }

  if (!subdivided) {
    Subdivide(contained);
    contained.clear();
  }

  for (auto &&child : children) {
    child->Insert(comp, depth + 1);
  }
}

void QuadTree::Subdivide(std::vector<Component *> &cont) {
  subdivided = true;
  used = false;
  for (int i = 0; i < NUM_CHILDREN; i++) {
    for (auto &&con : cont) {
      children[i]->Insert(con, MAX_DEPTH);
    }
  }
}

void QuadTree::Draw(raylib::Color quadColor, raylib::Color collColor) {
  if (subdivided)
    for (auto &&child : children) {
      child->Draw(quadColor, collColor);
    }
  else {
    if (contained.size() != 0) {
      Color col = {quadColor};
      DrawText(std::to_string(myNum).c_str(), collision.GetRect().x,
               collision.GetRect().y, 20, col);
      collision.DrawDebug(quadColor);
    }
  }
}

void QuadTree::ClearNode() {
  used       = false;
  subdivided = false;
  contained.clear();
}

QuadTree::~QuadTree() {

  ClearNode();
  for (auto &&child : children)
    child->ClearNode();
  currQuads = 0;
}

void QuadTree::CleanUp() {
    delete collision.position;
    for (auto &&child : children)
      child->CleanUp();
}
} // namespace pong