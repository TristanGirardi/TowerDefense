#ifndef CRITTER_OBSERVER_H
#define CRITTER_OBSERVER_H

#include "Observer.h"

class CritterGroup;

// The "view" for assignment 2 part 3. We keep it simple: when the wave
// changes, this prints each critter's stats to the console so you can see
// the observer pattern working without a second game window.
class CritterObserver : public Observer
{
public:
    explicit CritterObserver(CritterGroup &group);

    void update() override;

private:
    CritterGroup *group;
};

// Hooks an observer to a group, adds/removes critters, and shows the prints
// from update(). Called from Driver.cpp after the unit-style tests.
void runCritterObserverDemo();

#endif
