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
    // Initialize the map with SCENERY tiles
    map.resize(height, std::vector<TileType>(width, TileType::SCENERY));
}

// Function to set a tile at a specific position
void Map::setTile(int x, int y, TileType type)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        map[y][x] = type;
    }
    else
    {
        std::cerr << "Error: Coordinates out of bounds." << std::endl;
    }
}

// Function to get the tile type at a specific position
TileType Map::getTile(int x, int y) const
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        return map[y][x];
    }
    else
    {
        std::cerr << "Error: Coordinates out of bounds." << std::endl;
        return TileType::SCENERY; // Return SCENERY as a default value
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
    for (const std::vector<TileType> &row : map)
    {
        for (TileType tile : row)
        {
            switch (tile)
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

// function to make sure only one entry exists, one exit exists and that entry is connected to exit using a path
bool Map::validate() const
{
    // count entries and exits
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

    // keep track of cells already visited
    std::vector<std::vector<bool>> visited(
        height,
        std::vector<bool>(width, false));

    // DFS function
    std::function<bool(int, int)> dfs =
        [&](int x, int y) -> bool
    {
        // outside map
        if (x < 0 || x >= width ||
            y < 0 || y >= height)
        {
            return false;
        }

        // already visited
        if (visited[y][x])
        {
            return false;
        }

        // cannot travel through scenery
        if (map[y][x] == TileType::SCENERY)
        {
            return false;
        }

        // found exit
        if (map[y][x] == TileType::EXIT)
        {
            return true;
        }

        visited[y][x] = true;

        // check all 4 directions
        return dfs(x + 1, y) ||
               dfs(x - 1, y) ||
               dfs(x, y + 1) ||
               dfs(x, y - 1);
    };

    // start DFS from entry
    if (!dfs(entryX, entryY))
    {
        std::cerr << "Error: No valid path exists from entry to exit." << std::endl;
        return false;
    }

    return true;
}