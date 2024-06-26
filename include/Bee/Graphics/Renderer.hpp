/**
 * @file Renderer.hpp
 */

#pragma once

#include <string>

#include "Bee/Math/Vector2f.hpp"
#include "Bee/Math/Vector2i.hpp"

/**
 * @namespace Renderer
 * 
 * @brief All the renderer and window related functions.
 * 
 */
namespace Renderer
{
    /**
     * @brief Unload all loaded fonts.
     * 
     */
    void unloadAllFonts();

    /**
     * @brief Unload all loaded textures.
     * 
     */
    void unloadAllTextures();

    /**
     * @brief Get the position of the camera in the world.
     * 
     * @return the camera position in world coordinates
     */
    Vector2f getCameraPosition();

    /**
     * @brief Get the size of the viewport.
     * 
     * @return the size of the viewport
     */
    Vector2f getViewPortSize();

    /**
     * @brief Get the size of the internal screen. 
     * 
     * @return the size of the internal screen
     */
    Vector2i getScreenSize();

    /**
     * @brief Get the size of the window
     * 
     * @return the size of the window
     */
    Vector2i getWindowSize();

    /**
     * @brief Set the window to fullscreen or windowed mode.
     * 
     * @param fullscreen true for fullscreen and false for windowed mode
     */
    void setFullscreen(bool fullscreen);

    /**
     * @brief Set the icon of the window.
     * 
     * @param path a file path from where to load the image from
     */
    void setWindowIcon(const std::string& path);

    /**
     * @brief Set the title of the Window
     * 
     * @param title the title of the window
     */
    void setWindowTitle(const std::string& title);

    /**
     * @brief Set the position of the camera
     * 
     * @param x the x position of the camera in world coordinates
     * @param y the y position of the camera in world coordinates
     */
    void setCameraPosition(float x, float y);

    /**
     * @brief Set the position of the camera
     * 
     * @param cameraPosition the position of the camera in world coordinates
     */
    void setCameraPosition(const Vector2f& cameraPosition);

    /**
     * @brief Set the size of the viewport
     * 
     * @param width the width of the viewport
     * @param height the height of the viewport
     */
    void setViewportSize(float width, float height);

    /**
     * @brief Set the size of the viewport
     * 
     * @param viewportSize the size of the viewport
     */
    void setViewportSize(const Vector2f& viewportSize);
};