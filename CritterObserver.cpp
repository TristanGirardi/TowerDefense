#include "CritterObserver.h"

#include "CritterGroup.h"
#include "Map.h"

#include <iostream>

CritterObserver::CritterObserver(CritterGroup &group) : group(&group)
{
}

void CritterObserver::update()
{
    // Called automatically after addCritter, removeCritter, or spawnNext.
    std::cout << "CritterObserver: wave changed (" << group->size()
              << " critters, " << group->getNextCritterIndex() << " on map)"
              << std::endl;

    const std::vector<Critter> &critters = group->getCritters();
    for (std::size_t i = 0; i < critters.size(); ++i)
    {
        const Critter &c = critters[i];
        std::cout << "  critter " << i << ": level " << c.getLevel() << ", HP "
                  << c.getHitPoints() << ", at (" << c.getPosition().x << ", "
                  << c.getPosition().y << ")" << std::endl;
    }
    std::cout << std::endl;
}

void runCritterObserverDemo()
{
    std::cout << "========== Critter observer demo ==========" << std::endl;

    Map map(4, 2);
    map.setTile(0, 0, TileType::ENTRY);
    map.setTile(1, 0, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(3, 0, TileType::EXIT);

    CritterGroup group;
    CritterObserver observer(group);
    group.attach(&observer);

    Critter first(4, 20, 2, 1.0, 1);
    Critter second(6, 25, 2, 1.0, 2);

    std::cout << "Adding two critters to the wave..." << std::endl;
    group.addCritter(first);
    group.addCritter(second);

    std::cout << "Spawning the first critter on the entry..." << std::endl;
    group.spawnNext(map);

    std::cout << "Removing the last critter..." << std::endl;
    group.removeCritter(group.size() - 1);

    group.detach(&observer);
    std::cout << "Critter observer demo finished." << std::endl << std::endl;
}
