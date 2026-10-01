#include "CritterGroup.h"

#include <stdexcept>
#include <utility>

CritterGroup::CritterGroup() : nextCritterIndex(0)
{
}

CritterGroup::CritterGroup(std::vector<Critter> critters)
    : critters(std::move(critters)), nextCritterIndex(0)
{
}

bool CritterGroup::empty() const
{
    return critters.empty();
}

std::size_t CritterGroup::size() const
{
    return critters.size();
}

std::size_t CritterGroup::getNextCritterIndex() const
{
    return nextCritterIndex;
}

const std::vector<Critter> &CritterGroup::getCritters() const
{
    return critters;
}

std::vector<Critter> &CritterGroup::getCritters()
{
    return critters;
}

Critter *CritterGroup::spawnNext(const Map &map)
{
    if (nextCritterIndex >= critters.size())
    {
        return nullptr;
    }
    if (map.getEntryX() < 0 || map.getEntryY() < 0)
    {
        throw std::invalid_argument("The map must have an entry point.");
    }

    Critter &critter = critters[nextCritterIndex++];
    critter = Critter(critter.getReward(), critter.getHitPoints(),
                      critter.getStrength(), critter.getSpeed(),
                      critter.getLevel(), {map.getEntryX(), map.getEntryY()});
    return &critter;
}

Critter *CritterGroup::getNextCritter()
{
    for (std::size_t index = 0; index < nextCritterIndex; ++index)
    {
        if (critters[index].isAlive())
        {
            return &critters[index];
        }
    }
    return nullptr;
}

void CritterGroup::moveCritters(const Map &map)
{
    for (std::size_t index = 0; index < nextCritterIndex; ++index)
    {
        critters[index].move(map);
    }
}

int CritterGroup::attack(std::size_t index, int damage)
{
    if (index >= critters.size())
    {
        throw std::out_of_range("Critter index is out of range.");
    }

    critters[index].takeDamage(damage);
    return critters[index].claimReward();
}

int CritterGroup::collectExitLoss()
{
    int loss = 0;
    for (Critter &critter : critters)
    {
        loss += critter.claimCoinsStolenAtExit();
    }
    return loss;
}
