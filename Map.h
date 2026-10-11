#ifndef MAP_H
#define MAP_H

#include <vector>

// Towers go on scenery, critters walk on path. Entry and exit are path cells
// too, there's just one of each.
enum class TileType
{
    SCENERY,
    PATH,
    ENTRY,
    EXIT
};

// A width x height grid of tiles. (0, 0) is the top-left corner.
// The map doesn't know about towers or critters, they just ask it questions.
class Map
{
private:
    std::vector<std::vector<TileType>> map; // indexed map[y][x]

    int width;
    int height;

public:
    // Blank map, all scenery. Throws if a size is 0 or negative.
    Map(int width, int height);

    // Out of bounds prints an error instead of crashing.
    void setTile(int x, int y, TileType type);
    TileType getTile(int x, int y) const; // SCENERY if out of bounds

    // One entry, one exit, and a path connecting them.
    bool validate() const;

    int getWidth() const;
    int getHeight() const;

    // -1 if there isn't one yet
    int getEntryX() const;
    int getEntryY() const;
    int getExitX() const;
    int getExitY() const;

    // S = scenery, P = path, E = entry, X = exit
    void display() const;
};

#endif
