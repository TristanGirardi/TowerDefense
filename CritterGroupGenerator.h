#ifndef CRITTER_GROUP_GENERATOR_H
#define CRITTER_GROUP_GENERATOR_H

#include "CritterGroup.h"

// Called at the start of each wave to build its critters. You give it the
// wave 1 stats and later waves are scaled up from those.
class CritterGroupGenerator
{
private:
    int baseCritterCount;
    int baseHitPoints;
    int baseStrength;
    double baseSpeed;
    int baseReward;

public:
    CritterGroupGenerator(int baseCritterCount = 5,
                          int baseHitPoints = 10,
                          int baseStrength = 1,
                          double baseSpeed = 1.0,
                          int baseReward = 2);

    // waveLevel starts at 1
    CritterGroup generate(int waveLevel) const;
};

#endif
