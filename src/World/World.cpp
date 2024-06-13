#include "World.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <tinyxml2.h>

#include "Bee.hpp"
#include "Entity.hpp"
#include "Log.hpp"
#include "Collision/Collision.hpp"
#include "Collision/Intersection.hpp"
#include "Graphics/Renderer.hpp"

World::World() = default;

void World::updateInternal()
{
    for (const TileLayer &layer : layers)
    {
        for (int i = 0; i < worldHeight; i++)
        {
            for (int j = 0; j < worldWidth; j++)
            {
                const int tileId = layer.tileIds[j + i * worldWidth];
                if (tileId == 0) continue;

                Vector2i pos;
                pos.x = j;
                pos.y = i;

                SDL_Rect srcRect;
                srcRect.h = tiles[tileId].height;
                srcRect.w = tiles[tileId].width;
                srcRect.x = tiles[tileId].currentX;
                srcRect.y = tiles[tileId].currentY;

                Renderer::drawTile(pos, &srcRect, tiles[tileId].texture);
            }
        }
    }

    for (Tile &tile : tiles)
    {
        if (tile.animated && tile.animationFrames[tile.animationIndex].duration + tile.frameStartTime <= Bee::getTime())
        {
            tile.frameStartTime = Bee::getTime();
            tile.animationIndex++;
            if (tile.animationIndex >= tile.animationFrames.size())
            {
                tile.animationIndex = 0;
            }
            tile.currentX = tile.animationFrames[tile.animationIndex].tileId % tile.columns * tile.width;
            tile.currentY = tile.animationFrames[tile.animationIndex].tileId / tile.columns * tile.height;
        }
    }

    for (size_t i = 0; i < entities.size(); i++)
    {
        entities[i]->updateInternal();
        entities[i]->update();
    }

    for (const TileLayer &layer : foregroundLayers)
    {
        for (int i = 0; i < worldHeight; i++)
        {
            for (int j = 0; j < worldWidth; j++)
            {
                const int tileId = layer.tileIds[j + i * worldWidth];
                if (tileId == 0) continue;

                Vector2i pos;
                pos.x = j;
                pos.y = i;

                SDL_Rect srcRect;
                srcRect.h = tiles[tileId].height;
                srcRect.w = tiles[tileId].width;
                srcRect.x = tiles[tileId].currentX;
                srcRect.y = tiles[tileId].currentY;

                Renderer::drawTile(pos, &srcRect, tiles[tileId].texture);
            }
        }
    }

    for (size_t i = 0; i < hudObjects.size(); i++)
    {
        hudObjects[i]->update();
        hudObjects[i]->updateInternal();
    }
}

void World::addEntity(Entity* entity)
{
    if (std::find(entities.begin(), entities.end(), entity) == entities.end())
    {
        entities.push_back(entity);
    }
    else
    {
        Log::write("World", LogLevel::warning, "Entity already in world");
    }
}

Entity* World::getEntityByName(const std::string& name)
{
    for (Entity* entity : entities)
    {
        if (entity->getName() == name)
            return entity;
    }

    return nullptr;
}

std::vector<Entity*> World::getEntitiesByName(const std::string& name)
{
    std::vector<Entity*> matchingEntities;

    for (Entity* entity : entities)
    {
        if (entity->getName() == name)
            matchingEntities.push_back(entity);
    }

    return matchingEntities;
}

std::vector<Entity*> World::getAllEntities()
{
    return entities;
}

Entity* World::removeEntity(Entity* entity)
{
    if (std::find(entities.begin(), entities.end(), entity) != entities.end())
    {
        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
        return entity;
    }

    return nullptr;
}

void World::deleteAllEntities()
{
    for (const Entity* entity : entities)
    {
        delete entity;
    }

    entities.clear();
}

void World::addHUDObject(HUDObject* hudObject)
{
    if (std::find(hudObjects.begin(), hudObjects.end(), hudObject) == hudObjects.end())
    {
        hudObjects.push_back(hudObject);
    }
    else
    {
        Log::write("World", LogLevel::warning, "HUD Object already in world");
    }
}

std::vector<HUDObject*> World::getAllHUDObjects()
{
    return hudObjects;
}

HUDObject* World::removeHUDObject(HUDObject* hudObject)
{
    if (std::find(hudObjects.begin(), hudObjects.end(), hudObject) != hudObjects.end())
    {
        hudObjects.erase(std::remove(hudObjects.begin(), hudObjects.end(), hudObject), hudObjects.end());
        return hudObject;
    }

    return nullptr;
}

void World::deleteAllHUDObjects()
{
    for (const HUDObject* hudObject : hudObjects)
    {
        delete hudObject;
    }

    hudObjects.clear();
}

std::string World::getTileData(const Vector2f& position, const std::string& index) const
{
    if (static_cast<int>(position.x) < 0) return "";
    if (static_cast<int>(position.y) < 0) return "";
    if (static_cast<int>(position.x) > worldWidth) return "";
    if (static_cast<int>(position.y) > worldHeight) return "";

    int tileId = 0;

    for (const TileLayer& layer : layers)
    {
        if (const int tileIdT = layer.tileIds[static_cast<int>(position.x) + static_cast<int>(position.y) * worldWidth]; tileIdT != 0)
            tileId = tileIdT;
    }

    if (tiles[tileId].data.find(index) == tiles[tileId].data.end())
        return "";

    return tiles[tileId].data.at(index);
}

std::vector<Intersection> World::getIntersections(const Entity* entity) const
{
    std::vector<Intersection> intersections;

    for (Entity* entityLoop : entities)
    {
        if (entity == entityLoop) continue;

        Intersection intersection;
        intersection.entity = entityLoop;
        intersection.worldObject = nullptr;
        if (Collision::checkCollision(entity->getHitBox(), entityLoop->getHitBox(), intersection))
        {
            intersections.push_back(intersection);
        }
    }

    for (WorldObject* worldObject : worldObjects)
    {
        Intersection intersection;
        intersection.entity = nullptr;
        intersection.worldObject = worldObject;
        if (Collision::checkCollision(entity->getHitBox(), worldObject->getHitbox(), intersection))
        {
            intersections.push_back(intersection);
        }
    }

    return intersections;
}

void World::loadTileset(const std::string &source, int firstId)
{
    const std::string tileSetPath = "./assets/Worlds/" + source;

    tinyxml2::XMLDocument tilesetXML;
    tilesetXML.LoadFile(tileSetPath.c_str());
    if (tilesetXML.Error())
    {
        Log::write("World", LogLevel::error, "Can't not load tileset: %s / %s", source.c_str(), tilesetXML.ErrorName());
        return;
    }

    tinyxml2::XMLElement* tilesetXMLElement = tilesetXML.FirstChildElement("tileset");
    tinyxml2::XMLElement* imageXMLElement = tilesetXMLElement->FirstChildElement("image");

    int width = tilesetXMLElement->IntAttribute("tilewidth");
    int height = tilesetXMLElement->IntAttribute("tileheight");
    int columns = tilesetXMLElement->IntAttribute("columns");
    int tileCount = tilesetXMLElement->IntAttribute("tilecount");
    std::filesystem::path tilesetTexturePath = imageXMLElement->Attribute("source");
    SDL_Texture* texture = Renderer::loadTexture(tilesetTexturePath.replace_extension().string(), "./assets/Worlds/Tilesets/" + tilesetTexturePath.replace_extension().string() + ".png");

    for (int id = 0; id < tileCount; id++)
    {
        Tile tile;
        tile.animated = false;
        tile.animationIndex = 0;
        tile.frameStartTime = 0;
        tile.columns = columns;
        tile.width = width;
        tile.height = height;
        tile.tilesetWidth = imageXMLElement->IntAttribute("width");
        tile.tilesetHeight = imageXMLElement->IntAttribute("height");
        tile.texture = texture;
        tile.x = id % tile.columns * tile.width;
        tile.y = id / tile.columns * tile.height;
        tile.currentX = tile.x;
        tile.currentY = tile.y;

        tiles.insert(tiles.begin() + id + firstId, tile);
    }

    for (const tinyxml2::XMLElement* tileXMLElement = tilesetXMLElement->FirstChildElement("tile"); tileXMLElement != nullptr; tileXMLElement = tileXMLElement->NextSiblingElement("tile"))
    {
        int id = tileXMLElement->IntAttribute("id");

        if (const char* tileType = tileXMLElement->Attribute("type"))
            tiles[id + firstId].data.insert({"type", tileType});

        if (const tinyxml2::XMLElement* propertiesXMLElement = tileXMLElement->FirstChildElement("properties"))
        {
            for (const tinyxml2::XMLElement* propertyXMLElement = propertiesXMLElement->FirstChildElement("property"); propertyXMLElement != nullptr; propertyXMLElement = propertyXMLElement->NextSiblingElement("property"))
            {
                tiles[id + firstId].data.insert({propertyXMLElement->Attribute("name"), propertyXMLElement->Attribute("value")});
            }
        }

        if (const tinyxml2::XMLElement* animationXML = tileXMLElement->FirstChildElement("animation"))
        {
            for (const tinyxml2::XMLElement* animElement = animationXML->FirstChildElement("frame"); animElement != nullptr; animElement = animElement->NextSiblingElement("frame"))
            {
                tiles[id + firstId].animated = true;

                AnimationTileFrame frame{};
                frame.duration = animElement->IntAttribute("duration");
                frame.tileId = animElement->IntAttribute("tileid");
                tiles[id + firstId].animationFrames.push_back(frame);
            }
        }
    }
    Log::write("World", LogLevel::info, "Loaded %s tileset", tilesetTexturePath.replace_extension().string().c_str());
}

void World::loadTilemap(const std::string& tilemapName)
{
    const std::string tileMapPath = "./assets/Worlds/" + tilemapName + ".tmx";

    tiles.clear();
    layers.clear();
    foregroundLayers.clear();
    worldObjects.clear();

    tinyxml2::XMLDocument tilemapXML;
    tilemapXML.LoadFile(tileMapPath.c_str());
    if (tilemapXML.Error())
    {
        Log::write("World", LogLevel::error, "Can't load tilemap: %s / %s", tilemapName.c_str(), tilemapXML.ErrorName());
        return;
    }

    tinyxml2::XMLElement* mapXMLElement = tilemapXML.FirstChildElement("map");
    worldWidth = mapXMLElement->IntAttribute("width");
    worldHeight = mapXMLElement->IntAttribute("height");
    float tileWidth = mapXMLElement->IntAttribute("tilewidth");
    float tileHeight = mapXMLElement->IntAttribute("tileheight");

    for (const tinyxml2::XMLElement* element = mapXMLElement->FirstChildElement("layer"); element != nullptr; element = element->NextSiblingElement("layer"))
    {
        TileLayer layer;
        layer.name = element->Attribute("name");

        const char* layerDataStr = element->FirstChildElement("data")->GetText();
        std::stringstream layerData(layerDataStr);

        while (layerData.good())
        {
            std::string substr;
            getline(layerData, substr, ',');
            layer.tileIds.push_back(stoi(substr));
        }

        if (const char* layerClass = element->Attribute("class"); layerClass && !strcmp("foreground", layerClass))
        {
            foregroundLayers.push_back(layer);
        }
        else
        {
            layers.push_back(layer);
        }
    }

    Tile nullTile;
    nullTile.animated = false;
    nullTile.height = 0;
    nullTile.width = 0;
    nullTile.x = 0;
    nullTile.y = 0;
    nullTile.currentX = 0;
    nullTile.currentY = 0;
    tiles.push_back(nullTile);

    for (const tinyxml2::XMLElement* element = mapXMLElement->FirstChildElement("tileset"); element != nullptr; element = element->NextSiblingElement("tileset"))
    {
        int firstId = element->IntAttribute("firstgid");
        std::string source = element->Attribute("source");
        loadTileset(source, firstId);
    }

    for (tinyxml2::XMLElement* objectGroup = mapXMLElement->FirstChildElement("objectgroup"); objectGroup != nullptr; objectGroup = objectGroup->NextSiblingElement())
    {
        for (tinyxml2::XMLElement* object = objectGroup->FirstChildElement("object"); object != nullptr; object = object->NextSiblingElement())
        {
            WorldObject* worldObject = new WorldObject;
            const char* name = object->Attribute("name");
            const char* type = object->Attribute("type");
            float x = object->FloatAttribute("x") / tileWidth;
            float y = object->FloatAttribute("y") / tileHeight;
            float width = object->FloatAttribute("width") / tileWidth;
            float height = object->FloatAttribute("height") / tileHeight;

            if (name) worldObject->setData("name", name);
            if (type) worldObject->setData("type", type);

            if (tinyxml2::XMLElement* properties = object->FirstChildElement("properties"))
            {
                for (tinyxml2::XMLElement* property = properties->FirstChildElement("property"); property != nullptr; property = property->NextSiblingElement("property"))
                {
                    worldObject->setData(property->Attribute("name"), property->Attribute("value"));
                }
            }

            Hitbox hitbox;
            tinyxml2::XMLElement* polygon = object->FirstChildElement("polygon");
            tinyxml2::XMLElement* ellipse = object->FirstChildElement("ellipse");
            tinyxml2::XMLElement* point = object->FirstChildElement("point");

            if (polygon)
            {
                std::vector<Vector2f> polygonPoints;
                std::stringstream polygonPointsStr(polygon->Attribute("points"));

                while (polygonPointsStr.good())
                {
                    Vector2f polygonPoint;
                    std::string substr;

                    getline(polygonPointsStr, substr, ',');
                    polygonPoint.x = stof(substr) / tileWidth;
                    getline(polygonPointsStr, substr, ' ');
                    polygonPoint.y = stof(substr) / tileHeight;
                    polygonPoints.push_back(polygonPoint);
                }

                for (const Vector2f& polygonPoint : polygonPoints)
                {
                    hitbox.hitboxVertices.push_back(polygonPoint + Vector2f(x, y));
                }
            }
            else if (ellipse)
            {

            }
            else if (point)
            {

            }
            else
            {
                hitbox.hitboxVertices.emplace_back(x, y);
                hitbox.hitboxVertices.emplace_back(x, y + height);
                hitbox.hitboxVertices.emplace_back(x + width, y);
                hitbox.hitboxVertices.emplace_back(x + width, y + height);
            }
            worldObject->setHitbox(hitbox);
            worldObjects.push_back(worldObject);
        }
    }
    Log::write("World", LogLevel::info, "Loaded %s tilemap", tilemapName.c_str());
}

World::~World()
{
    for (const WorldObject* worldObject : worldObjects)
    {
        delete worldObject;
    }
}