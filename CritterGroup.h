#ifndef CRITTER_GROUP_H
#define CRITTER_GROUP_H

#include "Critter.h"

#include <cstddef>
#include <vector>

class CritterGroup
{
private:
    std::vector<Critter> critters;
    std::size_t nextCritterIndex;

public:
    CritterGroup();
    explicit CritterGroup(std::vector<Critter> critters);

    bool empty() const;
    std::size_t size() const;
    std::size_t getNextCritterIndex() const;
    const std::vector<Critter> &getCritters() const;
    std::vector<Critter> &getCritters();

    // Spawns the next critter at the map entry, preserving group order.
    Critter *spawnNext(const Map &map);
    Critter *getNextCritter();

    // Advances every active critter by one turn.
    void moveCritters(const Map &map);

    // Applies damage and returns the newly earned reward, if any.
    int attack(std::size_t index, int damage);

    // Returns the total coin loss caused by critters that reached the exit.
    int collectExitLoss();
};

#endif
