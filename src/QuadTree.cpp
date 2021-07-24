#include "QuadTree.h"
#include <vector>
#include <iostream>
#include "TUtils.h"
#define MAX_DEPTH 5
namespace pong
{
    int QuadTree::currQuads = 0;

    std::tuple<float, float, float, float> GetChildCoords(RectCollision *rect, int childIndex)
    {
        float xc, yc,
            x = rect->position->x, y = rect->position->y,
            w = rect->width, h = rect->height;
        switch (childIndex)
        {

        case 0:
        {
            xc = x, yc = y;
        }
        break;

        case 1:
        {
            xc = x + w / 2;
            yc = y;
        }
        break;

        case 2:
        {
            xc = x;
            yc = y + h / 2;
        }
        break;

        case 3:
        {
            xc = x + w / 2;
            yc = y + h / 2;
        }
        break;
        }
        return {xc, yc, w / 2, h / 2};
    }

    // Every quadtree node should check which collisions in the given vector are inside it
    // TODO: Don't store collisions if you have any children
    QuadTree::QuadTree(RectCollision *myColl, QuadTree *par, std::vector<Component *> *colliders, int depth) : thisColl(myColl)
    {
        parent = par;
        if (!colliders)
        {
            TraceLog(LOG_FATAL, "QUADTREE GIVEN NO COLLIDERS");
        }
        if (!par)
        {
            for (auto &&coll : *colliders)
            {
                TUtils::GetTypePtr<BaseCollision>(coll)->currentNode = nullptr;
            }
        }
        // If we've reached recursion limit
        if (depth == MAX_DEPTH)
        {
            objsInNode = colliders;
            for (auto &&coll : *objsInNode)
                dynamic_cast<BaseCollision *>(coll)->currentNode = this;

            delete parent->objsInNode;
            parent->objsInNode = nullptr;

            if (!colliders)
            {
                TraceLog(LOG_FATAL, "QUADTREE GIVEN NO COLLIDERS");
            }
            myNum = currQuads;
            currQuads++;
            return;
        }

        std::vector<RectCollision *> *childrenRects = new std::vector<RectCollision *>(QuadTree::maxChildren);
        std::vector<std::vector<Component *> *> *childrenColls = new std::vector<std::vector<Component *> *>(QuadTree::maxChildren);

        for (int i = 0; i < childrenRects->size(); i++)
        {
            RectCollision *currChildRect = (*childrenRects)[i];
            std::vector<Component *> *currChildColliders = (*childrenColls)[i];
            currChildColliders = new std::vector<Component *>();

            // Create child rectangle
            auto [x, y, w, h] = GetChildCoords(myColl, i);
            currChildRect = new RectCollision(x, y, w, h);

            // Check for colliders
            for (auto &&coll : *colliders)
            {
                BaseCollision *bc = dynamic_cast<BaseCollision *>(coll);
                if (currChildRect->CheckCollision(coll))
                {
                    // Add to the list, set the pointer
                    currChildColliders->push_back(coll);
                    (dynamic_cast<BaseCollision *>(coll))->currentNode = this;
                }
            }

            // If the current child is colliding with something, recurse
            if (currChildColliders->size() > 0)
            {
                QuadTree *child = new QuadTree(currChildRect, this, currChildColliders, depth + 1);
                children.push_back(child);
                numChildren++;
            }
            // Otherwise, free the memory we allocated for this child
            else
            {
                delete currChildColliders;
                delete currChildRect;
            }
        }
    }

    void QuadTree::Draw(raylib::Color quadColor, raylib::Color collColor)
    {

        if (numChildren == 0)
        {
            DrawTextRec(raylib::Font(), std::to_string(myNum).c_str(), thisColl->GetRect(), 20, 1, true, quadColor);

            thisColl->DrawDebug(quadColor);
            // TODO: I shouldn't really need this
            // A quad with no objects should NOT exist
            if (objsInNode)
                for (auto &&coll : *objsInNode)
                {
                    dynamic_cast<BaseCollision *>(coll)->DrawDebug(collColor);
                    // coll->DrawDebug(collColor);
                }
        }

        for (auto &&child : children)
        {
            if (child)
                child->Draw(quadColor, collColor);
        }
    }

    QuadTree::~QuadTree()
    {
        for (auto &&child : children)
        {
            if (child)
            {
                delete child;
                child = nullptr;
            }
        }
        // To avoid nuking systemComponents when freeing the root node
        if (objsInNode && parent)
        {
            delete objsInNode;
            objsInNode = nullptr;
        }
        if (thisColl)
            delete thisColl;
        thisColl = nullptr;
        currQuads = 0;
    }

    void QuadTree::PrintDebug(int depth)
    {
        std::string p;

        for (int i = 0; i < depth; i++)
            p += '\t';

        p += std::to_string(myNum) + '\n';

        for (int i = 0; i < depth + 1; i++)
            p += '\t';

        std::cout << p;

        for (auto &&child : children)
            child->PrintDebug(++depth);
    }
}