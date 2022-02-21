#include "QuadTree.h"
#include <vector>
#include <iostream>
#include "TUtils.h"

#define MAX_DEPTH 4

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
    QuadTree::QuadTree(RectCollision &passedColl, int cap) : capacity(cap), collision(passedColl)
    {
        children = std::vector<QuadTree *>(NUM_CHILDREN, nullptr);
    }

    void QuadTree::Insert(Component *comp, int depth)
    {
        BallCollision *ball = TUtils::GetTypePtr<BallCollision>(comp);
        if (!collision.CheckCollision(comp))
            return;

        if (contained.size() < capacity && !subdivided || depth == MAX_DEPTH)
        {
            contained.push_back(comp);
            TUtils::GetTypePtr<BaseCollision>(comp)->currentNodes.push_back(this);
            myNum = currQuads;
            currQuads++;
            return;
        }

        if (!subdivided)
        {
            Subdivide(contained);
            contained.clear();
        }

        for (auto &&child : children)
        {
            child->Insert(comp, depth + 1);
        }
    }

    void QuadTree::Subdivide(std::vector<Component *> &cont)
    {
        subdivided = true;
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            auto [x, y, w, h] = GetChildCoords(&collision, i);
            raylib::Vector2 *vec = new raylib::Vector2(x, y);
            RectCollision childColl = RectCollision(vec,w, h);

            children[i] = new QuadTree(childColl, capacity);
            for (auto &&con : cont)
            {
                children[i]->Insert(con, MAX_DEPTH);
            }
        }
    }

    void QuadTree::Draw(raylib::Color quadColor, raylib::Color collColor)
    {
        if (subdivided)
            for (auto &&child : children)
            {
                child->Draw(quadColor, collColor);
            }
        else
        {
            if (contained.size() != 0)
            {
                Color col = {quadColor};
                DrawText(std::to_string(myNum).c_str(), collision.GetRect().x, collision.GetRect().y, 20, col);
                collision.DrawDebug(quadColor);
            }
        }
    }

    QuadTree::~QuadTree()
    {

        for (auto &&child : children)
            delete child;
        currQuads = 0;
        delete collision.position;
    }
}