#ifndef CRITTER_GROUP_H
#define CRITTER_GROUP_H

#include "Critter.h"

#include <cstddef>
#include <vector>

// One wave. All the critters are made up front, then let onto the map one at
// a time with spawnNext(). The vector never changes size after that, so it's
// fine for towers and the GUI to hold pointers into it.
class CritterGroup
{
private:
    std::vector<Critter> critters;
    std::size_t nextCritterIndex; // how many have spawned so far

public:
    CritterGroup();
    explicit CritterGroup(std::vector<Critter> critters);

    bool empty() const;
    std::size_t size() const;
    std::size_t getNextCritterIndex() const;
    const std::vector<Critter> &getCritters() const;
    std::vector<Critter> &getCritters();

    // Puts the next critter on the entry. nullptr when the wave is all out.
    Critter *spawnNext(const Map &map);

    // The critter in the lead (first one spawned that's still alive).
    Critter *getNextCritter();

    // Advances every active critter by one turn.
    void moveCritters(const Map &map);

    // Returns the reward if this hit killed it, 0 otherwise.
    int attack(std::size_t index, int damage);

    // Coins stolen by critters that reached the exit since the last call.
    int collectExitLoss();
};

#endif
