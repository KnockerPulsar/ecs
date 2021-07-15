#pragma once
#include "raymath.h"

namespace pong
{
    class Transform
    {
    private:
    public:
        Vector3 position = Vector3{1, 1, 1};
        Quaternion rotation = Quaternion{0, 0, 0, 0};
        Vector3 scale = Vector3{1, 1, 1};

        Transform() {}
        Transform(Vector3 pos, Quaternion rot, Vector3 sc) : position(pos), rotation(rot), scale(sc) {}
        Transform(Vector3 pos) : position(pos), rotation{0, 0, 0, 0}, scale{1, 1, 1} {}
        ~Transform() {}
    };
} // namespace pong
