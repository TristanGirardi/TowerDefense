
#ifndef CRITTER_OBSERVER_H
#define CRITTER_OBSERVER_H

#include "Observer.h"

class Critter;
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

private:
    CritterGroup *group = nullptr;
    Critter *critter = nullptr;

    // Counts how many notifications were received.
    int updateCount = 0;
};

// Demo in the driver:
// attach observers, modify critters, and display updates.
void runCritterObserverDemo();

#endif
