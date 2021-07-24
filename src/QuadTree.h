#include "../vendor/raylib-cpp/raylib-cpp.hpp"
#include <vector>
#include "components/BaseCollision.h"
#include "components/RectCollision.h"

// Description:
/*
    The root is given a list of all colliders in the scene.
    It then goes and creates 4 child rectangles, it then checks if each collider
    is colliding with that child. If so, the collider is added to that child's list.

    If a certain child is colliding with anything, the QuadTree node is created and 
    data is passed to it. (Colliders, its own collision, its parent)

    The parent node should then de-allocate its own collider list since it's not needed

    If a child is not colliding with any object, it's not created 
    If we reached some maximum recursion depth, no more children are created
    
*/

namespace pong
{
    class QuadTree
    {
    private:
        // 0 top left, 1 top right, 2 bot left, 3 bot right
        int numChildren = 0;
        static const int maxChildren = 4;
        static int currQuads;
        int myNum = 0;
        RectCollision *thisColl;
        QuadTree *parent = nullptr;
        std::vector<QuadTree *> children = std::vector<QuadTree *>(0);

    public:
        std::vector<Component *> *objsInNode = nullptr;

        QuadTree(RectCollision *, QuadTree *, std::vector<Component *> *, int depth = 0);
        ~QuadTree();
        void Draw(raylib::Color, raylib::Color);
        void PrintDebug(int );
    };
}