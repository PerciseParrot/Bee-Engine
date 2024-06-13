#pragma once

#include <vector>

#include "Math/Vector2f.hpp"

struct Hitbox
{
    Vector2f center;
    std::vector<Vector2f> hitboxVertices;
};