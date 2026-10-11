#include "Map.h"

#include <iostream>
#include <queue>
#include <utility>
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
}

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

// First check there's exactly one entry and one exit, then do a BFS from the
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

    // Same BFS idea the critters use, except here we only care whether the
    // exit gets reached at all.
    std::queue<std::pair<int, int>> pending;
    pending.push({entryX, entryY});
    visited[entryY][entryX] = true;

    // No diagonal moves, only the four neighbours.
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!pending.empty())
    {
        const int x = pending.front().first;
        const int y = pending.front().second;
        pending.pop();

        if (map[y][x] == TileType::EXIT)
        {
            return true;
        }

        for (int i = 0; i < 4; ++i)
        {
            const int nx = x + dx[i];
            const int ny = y + dy[i];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height)
            {
                continue; // off the edge
            }
            if (visited[ny][nx] || map[ny][nx] == TileType::SCENERY)
            {
                continue; // been here, or critters can't walk on it
            }

            visited[ny][nx] = true;
            pending.push({nx, ny});
        }
    }

    std::cerr << "Error: No valid path exists from entry to exit." << std::endl;
    return false;
}