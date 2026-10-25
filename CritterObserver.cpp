
#include "CritterObserver.h"

#include "Critter.h"
#include "CritterGroup.h"
#include "Map.h"

#include <iostream>
#include <vector>

// Constructor for observing a group of critters.
CritterObserver::CritterObserver(CritterGroup &group)
    : group(&group)
{
}

// Constructor for observing an individual critter.
CritterObserver::CritterObserver(Critter &critter)
    : critter(&critter)
{
}

// Returns the number of observer updates received.
int CritterObserver::getUpdateCount() const
{
    return updateCount;
}

void CritterObserver::update()
{
    ++updateCount;

    // Called when an individual critter moves or takes damage.
    if (critter != nullptr)
    {
        Position position = critter->getPosition();

        std::cout << "CritterObserver: individual critter changed"
                  << " (update #" << updateCount << ")"
                  << std::endl;

        std::cout << "  Level: " << critter->getLevel()
                  << ", HP: " << critter->getHitPoints()
                  << ", position: (" << position.x << ", "
                  << position.y << ")"
                  << ", alive: "
                  << (critter->isAlive() ? "yes" : "no")
                  << std::endl
                  << std::endl;

        return;
    }

    // Called automatically after
    // addCritter, removeCritter, or spawnNext.
    if (group != nullptr)
    {
        std::cout << "CritterObserver: wave changed"
                  << " (update #" << updateCount << ", "
                  << group->size() << " critters, "
                  << group->getNextCritterIndex() << " on map)"
                  << std::endl;

        const std::vector<Critter> &critters = group->getCritters();

        for (std::size_t i = 0; i < critters.size(); ++i)
        {
            const Critter &c = critters[i];
            Position position = c.getPosition();

            std::cout << "  critter " << i
                      << ": level " << c.getLevel()
                      << ", HP " << c.getHitPoints()
                      << ", at (" << position.x << ", "
                      << position.y << ")" << std::endl;
        }

        std::cout << std::endl;
    }
}

void runCritterObserverDemo()
{
    std::cout << "Critter observer demo"
              << std::endl;

    Map map(4, 2);
    map.setTile(0, 0, TileType::ENTRY);
    map.setTile(1, 0, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(3, 0, TileType::EXIT);

    // PART A: Individual Critter Observer
    std::cout << "\nIndividual Critter Observer: "
              << std::endl;

    Critter individual(4, 20, 2, 1.0, 1);
    CritterObserver individualObserver(individual);

    individual.attach(&individualObserver);

    std::cout << "Damaging critter by 5 HP: "
              << std::endl;
    individual.takeDamage(5);

    std::cout << "Moving critter"
              << std::endl;
    individual.move(map);

    std::cout << "Damaging critter by another 5 HP: "
              << std::endl;
    individual.takeDamage(5);

    std::cout << "Total individual observer updates: "
              << individualObserver.getUpdateCount()
              << std::endl;

    // Verify detaching stops notifications.
    individual.detach(&individualObserver);

    int updatesBefore = individualObserver.getUpdateCount();

    std::cout << "Damaging critter after detaching: "
              << std::endl;
    individual.takeDamage(2);

    std::cout << "Observer correctly detached: "
              << (individualObserver.getUpdateCount() == updatesBefore
                      ? "yes"
                      : "no")
              << std::endl;

    // PART B: Existing CritterGroup Observer
    std::cout << "\nCritterGroup Observer: "
              << std::endl;

    CritterGroup group;
    CritterObserver groupObserver(group);
    group.attach(&groupObserver);

    Critter first(4, 20, 2, 1.0, 1);
    Critter second(6, 25, 2, 1.0, 2);

    std::cout << "Adding two critters to the wave: "
              << std::endl;

    group.addCritter(first);
    group.addCritter(second);

    std::cout << "Spawning the first critter on the entry: "
              << std::endl;

    group.spawnNext(map);

    std::cout << "Removing the last critter: "
              << std::endl;

    group.removeCritter(group.size() - 1);

    std::cout << "Total group observer updates: "
              << groupObserver.getUpdateCount()
              << std::endl;

    group.detach(&groupObserver);

    std::cout << "Critter observer demo finished."
              << std::endl
              << std::endl;
}
