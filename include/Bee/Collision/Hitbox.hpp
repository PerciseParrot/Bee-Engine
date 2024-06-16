#pragma once

#include <vector>

#include "Bee/Math/Vector2f.hpp"

struct Hitbox
{
    Vector2f center;

    std::vector<Vector2f> vertices;

    bool isEllipse = false;
    Vector2f ellipse;
};