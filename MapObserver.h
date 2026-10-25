#ifndef MAP_OBSERVER_H
#define MAP_OBSERVER_H

#include "Map.h"
#include "Observer.h"

#include <SFML/Graphics.hpp>

/**
 * @brief The view of the map (Part 2).
 *
 * It is attached to a Map. Every time the map calls notify(), update()
 * prints the map in the console, and draw() shows it in an SFML window.
 */
class MapObserver : public Observer
{
public:
    /**
     * @brief Makes a view for a map.
     * @param map The map to show. The view only reads from it.
     */
    explicit MapObserver(const Map &map);

    /** @brief Called by the map every time it changes. Prints the new map. */
    void update() override;

    /**
     * @brief Draws the map, one square per tile, with the towers on top.
     * @param window Where to draw.
     */
    void draw(sf::RenderWindow &window) const;

    /** @brief How many times the map has notified this view. */
    int getUpdateCount() const;

private:
    const Map &map;  ///< The map being watched (read-only).
    Map lastSeen;    ///< Copy of the map from the last update(). draw() uses it.
    int updateCount; ///< Number of update() calls so far.
};

/**
 * @brief Part 2 driver. Creates a map, plugs a MapObserver into it, then
 * changes the map one step per Space press so you can watch the view update.
 */
void runMapObserverDemo();

#endif
