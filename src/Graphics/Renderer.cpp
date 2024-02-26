#include "Renderer.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Math/Vector2f.hpp"
#include "Math/Vector2i.hpp"

static int windowWidth = 0;
static int windowHeight = 0;
static float viewPortWidth = 16.0f;
static float viewPortHeight = 9.0f;
static SDL_Window* window;
static SDL_Renderer* renderer;
static std::unordered_map<std::string, SDL_Texture*> textureMap;
static Vector2f cameraPosition;

void Renderer::init(int winWidth, int winHeight)
{
    windowWidth = winWidth;
    windowHeight = winHeight;

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        std::cout << "Error initializing SDL2_image: " << SDL_GetError() << std::endl;
        throw std::exception();
    }

    window = SDL_CreateWindow("Bee Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
    if (window == nullptr)
    {
        std::cout << "Error creating Window: " << SDL_GetError() << std::endl;
        throw std::exception();
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr)
    {
        std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
        throw std::exception();
    }

    std::cout << "Initialized renderer" << std::endl;
}

void Renderer::clear()
{
    SDL_RenderClear(renderer);
}

void Renderer::display()
{
    SDL_RenderPresent(renderer);
}

void Renderer::drawTile(const Vector2i& position, SDL_Rect* srcRect, SDL_Texture* texture)
{
    SDL_Rect dstRect;
    dstRect.x = ceilf((position.x - cameraPosition.x + viewPortWidth / 2) * windowWidth / viewPortWidth);
    dstRect.y = ceilf((position.y - cameraPosition.y + viewPortHeight / 2) * windowHeight / viewPortHeight);
    dstRect.h = ceilf(windowHeight / viewPortHeight);
    dstRect.w = ceilf(windowWidth / viewPortWidth);

    SDL_RenderCopy(renderer, texture, srcRect, &dstRect);
}

void Renderer::drawSprite(const Vector2f& position, SDL_Rect* srcRect, SDL_Texture* texture, const Vector2f& scale, Vector2f& rotationCenter, float rotation)
{
    SDL_Rect dstRect;
    dstRect.x = (position.x - scale.x / 2 - cameraPosition.x + viewPortWidth / 2) * windowWidth / viewPortWidth;
    dstRect.y = (position.y - scale.y / 2 - cameraPosition.y + viewPortHeight / 2) * windowHeight / viewPortHeight;
    dstRect.h = windowHeight / viewPortHeight * scale.x;
    dstRect.w = windowWidth / viewPortWidth * scale.y;

    SDL_Point centerPoint;
    centerPoint.x = dstRect.w * rotationCenter.x;
    centerPoint.y = dstRect.h * rotationCenter.y;

    SDL_RenderCopyEx(renderer, texture, srcRect, &dstRect, rotation, &centerPoint, SDL_FLIP_NONE);
}

SDL_Texture* Renderer::loadTexture(std::string textureName, std::string path)
{
    SDL_Texture* texture = nullptr;

    if (textureMap.find(textureName) != textureMap.end())
        return textureMap[textureName];
    
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr)
    {
        std::cout << "Error loading image" << std::endl;
        throw std::exception();
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr)
    {
        std::cout << "Error creating texture: " << SDL_GetError() << std::endl;
        throw std::exception();
    }
    SDL_FreeSurface(surface);

    textureMap.insert(std::pair<std::string, SDL_Texture*>(textureName, texture));
    return texture;
}

void Renderer::unloadTexture(std::string textureName)
{
    SDL_DestroyTexture(textureMap.at(textureName));
    textureMap.erase(textureName);
}

void Renderer::unloadAllTextures()
{
    for (const auto& [key, val] : textureMap)
    {
        std::cout << "Unloaded " << key << " texture" << std::endl;
        SDL_DestroyTexture(val);
    }
    textureMap.clear();
}

void Renderer::setWindowIcon(std::string path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr)
    {
        std::cout << "Error loading image" << std::endl;
        throw std::exception();
    }
    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);
}

void Renderer::setWindowTitle(std::string title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

void Renderer::setCameraPosition(float x, float y)
{
    cameraPosition.x = x;
    cameraPosition.y = y;
}

void Renderer::setCameraPosition(const Vector2f& newCameraPosition)
{
    cameraPosition = newCameraPosition;
}

void Renderer::setViewportSize(float width, float height)
{
    viewPortWidth = width;  
    viewPortHeight = height;
}

void Renderer::setViewportSize(const Vector2f& viewportSize)
{
    viewPortWidth = viewportSize.x;
    viewPortHeight = viewportSize.y;  
}

void Renderer::cleanUp()
{
    unloadAllTextures();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
}