#ifndef CRITTER_GROUP_H
#define CRITTER_GROUP_H

#include "Critter.h"
#include "Observable.h"

#include <cstddef>
#include <vector>

// One wave of critters. spawnNext() puts them on the map one at a time.
// The whole wave is observable: add/remove critters here and CritterObserver
// gets notified (single Critter objects stay as they were in A1).
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
