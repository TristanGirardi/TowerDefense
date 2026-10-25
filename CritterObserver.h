
#ifndef CRITTER_OBSERVER_H
#define CRITTER_OBSERVER_H

#include "Critter.h"
#include "Observer.h"

#include <SFML/Graphics.hpp>

#include <string>

class CritterGroup;

// Observer view for a critter or a critter wave:
// prints stats whenever the observed object changes.
class CritterObserver : public Observer
{
public:
    // Observe an entire wave of critters.
    explicit CritterObserver(CritterGroup &group);

    // Observe an individual critter.
    explicit CritterObserver(Critter &critter);

    void update() override;

    // Track the number of observer updates.
    int getUpdateCount() const;

    // Draws the individual critter's panel (320 x 250 pixels) in an SFML
    // window. font can be nullptr to draw only the shapes.
    void draw(sf::RenderWindow &window, const sf::Font *font,
              sf::Vector2f topLeft) const;

private:
    CritterGroup *group = nullptr;
    Critter *critter = nullptr;

    // Counts how many notifications were received.
    int updateCount = 0;

    // The individual critter's values as of the last update().
    // draw() only uses these, so a detached view stops changing.
    void readCritter();
    int shownLevel = 0;
    int shownHitPoints = 0;
    int maxHitPoints = 0; // hit points at the start, for the health bar
    Position shownPosition = {0, 0};
    std::string shownStatus; // "Walking", "Killed" or "Reached the exit"
};

// Demo in the driver: a window for one critter (each Space press changes
// it once and the panel redraws), then a console demo for a wave.
void runCritterObserverDemo();

#endif
