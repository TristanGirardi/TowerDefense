#ifndef TOWER_OBSERVER_H
#define TOWER_OBSERVER_H

#include "Observer.h"
#include "Tower.h"

#include <SFML/Graphics.hpp>

#include <string>

/**
 * @brief The view of one tower (Part 1).
 */
class TowerObserver : public Observer
{
public:
    /**
     * @brief Makes a view for a tower and reads its starting values.
     * @param tower The tower to show. The view only reads from it.
     */
    explicit TowerObserver(const Tower &tower);

    /** @brief Called by the tower every time it changes. Re-reads its values. */
    void update() override;

    /**
     * @brief Draws the tower panel (320 x 250 pixels).
     * @param window Where to draw.
     * @param font Font for the text, or nullptr to draw only the shapes.
     * @param position Top-left corner of the panel, in pixels.
     */
    void draw(sf::RenderWindow &window, const sf::Font *font,
              sf::Vector2f position) const;

    /** @brief How many times the tower has notified this view. */
    int getUpdateCount() const;

private:
    /** @brief Copies the tower's current values into the fields below. */
    void readTower();

    const Tower &tower; ///< The tower being watched (read-only).
    int updateCount;    ///< Number of update() calls so far.

    // The tower's values as of the last update(). draw() only uses these.
    std::string name; // Tower type, e.g. "Direct Tower".
    int level;        // Current level, starting at 1.
    int maxLevel;     // Highest level for this tower type.
    float x;          // X value on the map.
    float y;          // Y value on the map.
    TowerStats stats; // range, power and fire rate at this level.
    int upgradeCost;  // Price of the next level, -1 at max level.
    int refundValue;  // Coins you get back for selling it.
};

/**
 * @brief Part 1 driver. Creates a tower, plugs a TowerObserver into it, then
 * changes the tower one step per Space press so you can watch the view update.
 */
void runTowerObserverDemo();

#endif
