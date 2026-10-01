#ifndef CRITTER_GROUP_GENERATOR_H
#define CRITTER_GROUP_GENERATOR_H

#include "CritterGroup.h"

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

    CritterGroup generate(int waveLevel) const;
    CritterGroup generateGroup(int waveLevel) const
    {
        return generate(waveLevel);
    }
};

#endif
