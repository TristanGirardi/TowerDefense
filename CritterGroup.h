#ifndef CRITTER_GROUP_H
#define CRITTER_GROUP_H

#include "Critter.h"
#include "Observable.h"

#include <cstddef>
#include <vector>

// One wave of critters. spawnNext() puts them on the map one at a time.
//
// Assignment 2 part 3: we treat the wave as the observable thing, not each
// Critter on its own — you add and remove critters from this group, and
// CritterObserver listens for those changes.
class CritterGroup : public Observable
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

    // Used by the part 3 driver demo; both call notify() when done.
    void addCritter(Critter critter);
    void removeCritter(std::size_t index);

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
