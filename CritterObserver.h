#ifndef CRITTER_OBSERVER_H
#define CRITTER_OBSERVER_H

#include "Observer.h"

class CritterGroup;

// Observer view for a critter wave: prints stats when the group changes.
class CritterObserver : public Observer
{
public:
    explicit CritterObserver(CritterGroup &group);

    void update() override;

private:
    CritterGroup *group;
};

// Small demo in the driver: attach, add/remove critters, watch console output.
void runCritterObserverDemo();

#endif
