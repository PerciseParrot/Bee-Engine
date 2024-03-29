#pragma once

#include <string>

#include "Bee/Math/Vector2f.hpp"
#include "Bee/Math/Vector2i.hpp"

namespace Renderer
{
    Vector2i getScreenSize();
    void setWindowIcon(std::string path);
    void setWindowTitle(std::string title);
    void setCameraPosition(float x, float y);
    void setCameraPosition(const Vector2f& cameraPosition);
    void setViewportSize(float width, float height);
    void setViewportSize(const Vector2f& viewportSize);
};