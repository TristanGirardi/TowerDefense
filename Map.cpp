#include "Map.h"

#include <iostream>
#include <functional>
#include <stdexcept>

Map::Map(int width, int height) : width(width), height(height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument(
            "Width and height must be greater than 0.");
    }
    // Start with an empty field of grass; the path gets drawn on afterwards.
    map.resize(height, std::vector<TileType>(width, TileType::SCENERY));
    towers.resize(height, std::vector<bool>(width, false));
}

void Map::setTile(int x, int y, TileType type)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        map[y][x] = type;
        if (type != TileType::SCENERY)
        {
            towers[y][x] = false; // a tower can't stay on the path
        }
        notify(); // the map changed, tell the observers
    }
    else
    {
        std::cerr << "Error: Coordinates out of bounds." << std::endl;
    }
}

TileType Map::getTile(int x, int y) const
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        return map[y][x];
    }
    else
    {
        std::cerr << "Error: Coordinates out of bounds." << std::endl;
        return TileType::SCENERY; // nothing can walk off the edge of the map
    }
}

int Map::getWidth() const
{
    return width;
}

int Map::getHeight() const
{
    return height;
}

// We just scan the grid for the entry/exit instead of storing them, so
// setTile() doesn't have anything to keep in sync.
int Map::getEntryX() const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (map[y][x] == TileType::ENTRY)
            {
                return x;
            }
        }
    }
    return -1; // Return -1 if no entry found
}

int Map::getEntryY() const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (map[y][x] == TileType::ENTRY)
            {
                return y;
            }
        }
    }
    return -1; // Return -1 if no entry found
}

int Map::getExitX() const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (map[y][x] == TileType::EXIT)
            {
                return x;
            }
        }
    }
    return -1; // Return -1 if no exit found
}

int Map::getExitY() const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (map[y][x] == TileType::EXIT)
            {
                return y;
            }
        }
    }
    return -1; // Return -1 if no exit found
}

void Map::display() const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (towers[y][x])
            {
                std::cout << "T ";
                continue;
            }
            switch (map[y][x])
            {
            case TileType::SCENERY:
                std::cout << "S ";
                break;
            case TileType::PATH:
                std::cout << "P ";
                break;
            case TileType::ENTRY:
                std::cout << "E ";
                break;
            case TileType::EXIT:
                std::cout << "X ";
                break;
            }
        }
        std::cout << std::endl;
    }
}

bool Map::hasTower(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return false;
    }
    return towers[y][x];
}

bool Map::placeTower(int x, int y)
{
    // Only on scenery that is inside the map and still empty.
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return false;
    }
    if (map[y][x] != TileType::SCENERY || towers[y][x])
    {
        return false;
    }
    towers[y][x] = true;
    notify();
    return true;
}

bool Map::removeTower(int x, int y)
{
    if (!hasTower(x, y))
    {
        return false;
    }
    towers[y][x] = false;
    notify();
    return true;
}

bool Map::moveTower(int fromX, int fromY, int toX, int toY)
{
    if (!hasTower(fromX, fromY))
    {
        return false; // no tower to move
    }
    if (toX < 0 || toX >= width || toY < 0 || toY >= height)
    {
        return false; // off the map
    }
    if (map[toY][toX] != TileType::SCENERY || towers[toY][toX])
    {
        return false; // can't go there
    }
    // Change both cells first, then notify once for the whole move.
    towers[fromY][fromX] = false;
    towers[toY][toX] = true;
    notify();
    return true;
}

// First check there's exactly one entry and one exit, then do a DFS from the
// entry over path cells to see if we can reach the exit.
bool Map::validate() const
{
    int entry = 0;
    int exit = 0;

    int entryX = -1;
    int entryY = -1;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (map[y][x] == TileType::ENTRY)
            {
                entry++;
                entryX = x;
                entryY = y;
            }
            else if (map[y][x] == TileType::EXIT)
            {
                exit++;
            }
        }
    }

    if (entry != 1 || exit != 1)
    {
        std::cerr << "Error: There must be exactly one entry and one exit." << std::endl;
        return false;
    }

    // Without this we'd walk back and forth between two path tiles forever.
    std::vector<std::vector<bool>> visited(
        height,
        std::vector<bool>(width, false));

    // We only care whether the exit can be reached at all, not how short the
    // route is, so a plain recursive DFS is enough here.
    std::function<bool(int, int)> dfs =
        [&](int x, int y) -> bool
    {
        if (x < 0 || x >= width || y < 0 || y >= height)
        {
            return false; // off the edge
        }
        if (visited[y][x] || map[y][x] == TileType::SCENERY)
        {
            return false; // been here, or critters can't walk on it
        }
        if (map[y][x] == TileType::EXIT)
        {
            return true;
        }

        visited[y][x] = true;

        // No diagonal moves, only the four neighbours.
        return dfs(x + 1, y) ||
               dfs(x - 1, y) ||
               dfs(x, y + 1) ||
               dfs(x, y - 1);
    };

    if (!dfs(entryX, entryY))
    {
        std::cerr << "Error: No valid path exists from entry to exit." << std::endl;
        return false;
    }

    return true;
}