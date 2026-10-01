#include "CritterGroupGenerator.h"

#include <stdexcept>
#include <utility>

CritterGroupGenerator::CritterGroupGenerator(int baseCritterCount,
                                             int baseHitPoints,
                                             int baseStrength,
                                             double baseSpeed,
                                             int baseReward)
    : baseCritterCount(baseCritterCount),
      baseHitPoints(baseHitPoints),
      baseStrength(baseStrength),
      baseSpeed(baseSpeed),
      baseReward(baseReward)
{
    if (baseCritterCount <= 0 || baseHitPoints <= 0 || baseStrength < 0 ||
        baseSpeed < 0.0 || baseReward < 0)
    {
        throw std::invalid_argument("Base critter statistics are invalid.");
    }
}

CritterGroup CritterGroupGenerator::generate(int waveLevel) const
{
    if (waveLevel <= 0)
    {
        throw std::invalid_argument("Wave level must be greater than zero.");
    }

    const int count = baseCritterCount + waveLevel - 1;
    const int hitPoints = baseHitPoints * waveLevel;
    const int strength = baseStrength + (waveLevel - 1) / 2;
    const double speed = baseSpeed * (1.0 + 0.05 * (waveLevel - 1));
    const int reward = baseReward * waveLevel;

    std::vector<Critter> critters;
    critters.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index)
    {
        critters.emplace_back(reward, hitPoints, strength, speed, waveLevel);
    }
    return CritterGroup(std::move(critters));
}
