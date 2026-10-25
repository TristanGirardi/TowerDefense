#ifndef MAP_H
#define MAP_H

#include "Observable.h"

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
// The map also remembers which scenery cells have a tower on them.
// It is Observable (Part 2): every change calls notify(), so a MapObserver
// can redraw the map.
class Map : public Observable
{
private:
    std::vector<std::vector<TileType>> map; // indexed map[y][x]
    std::vector<std::vector<bool>> towers;  // true where a tower stands

    int width;
    int height;

public:
    // Blank map, all scenery. Throws if a size is 0 or negative.
    Map(int width, int height);

    // Out of bounds prints an error instead of crashing. Notifies observers.
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

    // Towers can only go on empty scenery. These return false (and don't
    // notify) if the move isn't allowed.
    bool placeTower(int x, int y);
    bool moveTower(int fromX, int fromY, int toX, int toY);
    bool removeTower(int x, int y);
    bool hasTower(int x, int y) const;

    // S = scenery, P = path, E = entry, X = exit, T = tower
    void display() const;
};

#endif
