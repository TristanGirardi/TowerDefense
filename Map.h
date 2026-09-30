#ifndef MAP_H
#define MAP_H

#include <vector>

// Towers can go on sceneries, critters can move on paths, and critters can enter and exit the map through entry and exit tiles.
// Each cell in the map is represented by a TileType, which can be one of the following: SCENERY, PATH, ENTRY, or EXIT.
enum class TileType
{
    SCENERY,
    PATH,
    ENTRY,
    EXIT
};

class Map
{
private:
    // 2D vector to store the map tiles for the grid
    std::vector<std::vector<TileType>> map;

    // Store dimensions
    int width;
    int height;

public:
    Map(int width, int height);

    // Function to set a tile at a specific position
    void setTile(int x, int y, TileType type);

    // Function to get the tile type at a specific position
    TileType getTile(int x, int y) const;

    // Function to make sure only one entry exists, one exit exists and that entry is connected to exit using a path
    bool validate() const;

    // Function to get the width of the map
    int getWidth() const;

    // Function to get the height of the map
    int getHeight() const;

    int getEntryX() const;

    int getEntryY() const;

    int getExitX() const;

    int getExitY() const;

    // Function to display the map
    void display() const;
};

#endif