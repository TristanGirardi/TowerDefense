// Test driver. It first runs console tests for the Map (Part 1),
// Tower (Part 2) and Critter group generator (Part 3) classes, prints whether
// each check passed, and then opens the graphical game.

#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "CritterObserver.h"
#include "Gui.h"
#include "Map.h"
#include "Observer.h"
#include "Tower.h"
#include "TowerObserver.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
int passedChecks = 0;
int failedChecks = 0;

// Records one test result and prints it on its own line.
void check(bool condition, const std::string &description)
{
    if (condition)
    {
        ++passedChecks;
        std::cout << "  [PASS] " << description << std::endl;
    }
    else
    {
        ++failedChecks;
        std::cout << "  [FAIL] " << description << std::endl;
    }
}
}

void testMap()
{
    std::cout << "========== Part 1: Map ==========" << std::endl;

    // A valid map: the path winds from the entry (E) to the exit (X).
    Map validMap(6, 4);
    validMap.setTile(0, 0, TileType::ENTRY);
    validMap.setTile(1, 0, TileType::PATH);
    validMap.setTile(2, 0, TileType::PATH);
    validMap.setTile(2, 1, TileType::PATH);
    validMap.setTile(2, 2, TileType::PATH);
    validMap.setTile(3, 2, TileType::PATH);
    validMap.setTile(4, 2, TileType::PATH);
    validMap.setTile(4, 3, TileType::PATH);
    validMap.setTile(5, 3, TileType::EXIT);

    std::cout << "Valid map (" << validMap.getWidth() << "x"
              << validMap.getHeight() << "):" << std::endl;
    validMap.display();
    std::cout << "Entry at (" << validMap.getEntryX() << ", "
              << validMap.getEntryY() << "), exit at ("
              << validMap.getExitX() << ", " << validMap.getExitY() << ")"
              << std::endl;
    std::cout << "validate(): " << (validMap.validate() ? "valid" : "invalid")
              << std::endl
              << std::endl;

    // Invalid: the path is broken, so the entry cannot reach the exit.
    Map brokenPath(5, 1);
    brokenPath.setTile(0, 0, TileType::ENTRY);
    brokenPath.setTile(1, 0, TileType::PATH);
    brokenPath.setTile(3, 0, TileType::PATH);
    brokenPath.setTile(4, 0, TileType::EXIT);
    std::cout << "Map with a broken path:" << std::endl;
    brokenPath.display();
    std::cout << "validate(): " << (brokenPath.validate() ? "valid" : "invalid")
              << std::endl
              << std::endl;

    // Invalid: two entries.
    Map twoEntries(4, 1);
    twoEntries.setTile(0, 0, TileType::ENTRY);
    twoEntries.setTile(1, 0, TileType::ENTRY);
    twoEntries.setTile(2, 0, TileType::PATH);
    twoEntries.setTile(3, 0, TileType::EXIT);
    std::cout << "Map with two entries:" << std::endl;
    twoEntries.display();
    std::cout << "validate(): " << (twoEntries.validate() ? "valid" : "invalid")
              << std::endl
              << std::endl;

    // Invalid: no exit at all.
    Map noExit(3, 1);
    noExit.setTile(0, 0, TileType::ENTRY);
    noExit.setTile(1, 0, TileType::PATH);
    std::cout << "Map with no exit:" << std::endl;
    noExit.display();
    std::cout << "validate(): " << (noExit.validate() ? "valid" : "invalid")
              << std::endl
              << std::endl;

    // Out of bounds access and invalid dimensions are rejected.
    std::cout << "Setting a tile outside the map:" << std::endl;
    validMap.setTile(10, 10, TileType::PATH);
    std::cout << "Creating a 0x5 map:" << std::endl;
    bool rejectedBadSize = false;
    try
    {
        Map badSize(0, 5);
    }
    catch (const std::invalid_argument &error)
    {
        rejectedBadSize = true;
        std::cout << "Caught exception: " << error.what() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Map checks:" << std::endl;
    check(validMap.getWidth() == 6 && validMap.getHeight() == 4,
          "a blank map is created with the requested size");
    check(validMap.getTile(1, 0) == TileType::PATH &&
              validMap.getTile(0, 1) == TileType::SCENERY,
          "setTile() sets cells and untouched cells stay scenery");
    check(validMap.getEntryX() == 0 && validMap.getEntryY() == 0 &&
              validMap.getExitX() == 5 && validMap.getExitY() == 3,
          "entry and exit are found at the cells they were set on");
    check(validMap.validate(), "a connected entry-to-exit path is valid");
    check(!brokenPath.validate(), "a broken path is invalid");
    check(!twoEntries.validate(), "a map with two entries is invalid");
    check(!noExit.validate(), "a map with no exit is invalid");
    check(rejectedBadSize, "a map with a zero dimension is rejected");
    std::cout << std::endl;
}

namespace
{
void printTower(const Tower &tower)
{
    const TowerStats &stats = tower.getStats();
    std::cout << tower.getName() << " at (" << tower.getX() << ", "
              << tower.getY() << ") level " << tower.getLevel() << "/"
              << tower.getMaxLevel() << ": range " << stats.range
              << ", power " << stats.power << ", fire rate "
              << stats.fireRate << "/s, refund " << tower.getRefundValue()
              << std::endl;
}

void printCritters(const std::vector<Critter *> &critters)
{
    for (std::size_t i = 0; i < critters.size(); ++i)
    {
        const Critter *c = critters[i];
        std::cout << "  Critter " << i << " at (" << c->getPosition().x << ", "
                  << c->getPosition().y << ") level " << c->getLevel()
                  << " HP " << c->getHitPoints()
                  << (c->isAlive() ? "" : " (dead)") << std::endl;
    }
}
}

void testTower()
{
    std::cout << "========== Part 2: Tower ==========" << std::endl;

    // Buying, upgrading and selling.
    DirectTower direct(2.0f, 1.0f);
    int coins = 500;
    std::cout << "Starting coins: " << coins << std::endl;
    check(direct.getBuyCost() == 100 && direct.getLevel() == 1,
          "a new Direct Tower costs 100 and starts at level 1");
    coins -= direct.getBuyCost();
    std::cout << "Bought " << direct.getName() << " for "
              << direct.getBuyCost() << ", coins left: " << coins << std::endl;
    printTower(direct);

    while (direct.canUpgrade())
    {
        int cost = direct.getUpgradeCost();
        direct.upgrade();
        coins -= cost;
        std::cout << "Upgraded for " << cost << ", coins left: " << coins
                  << std::endl;
        printTower(direct);
    }
    std::cout << "canUpgrade() at max level: "
              << (direct.canUpgrade() ? "true" : "false")
              << ", getUpgradeCost(): " << direct.getUpgradeCost()
              << ", upgrade(): " << (direct.upgrade() ? "true" : "false")
              << std::endl;
    check(direct.getLevel() == direct.getMaxLevel() && !direct.canUpgrade() &&
              direct.getUpgradeCost() == -1 && !direct.upgrade(),
          "a tower at max level cannot be upgraded further");
    check(direct.getRefundValue() == (100 + 60 + 90 + 120) / 2,
          "selling refunds half of everything spent on the tower");
    coins += direct.getRefundValue();
    std::cout << "Sold tower for " << direct.getRefundValue()
              << ", coins now: " << coins << std::endl
              << std::endl;

    // Targeting and firing: a DirectTower hits the closest critter in range.
    Critter nearCritter(5, 30, 1, 1.0, 1, {2, 2});
    Critter farCritter(5, 30, 1, 1.0, 1, {4, 4});
    Critter outOfRange(5, 30, 1, 1.0, 1, {9, 9});
    std::vector<Critter *> critters = {&farCritter, &nearCritter, &outOfRange};

    DirectTower shooter(2.0f, 1.0f);
    printTower(shooter);
    std::cout << "Before firing:" << std::endl;
    printCritters(critters);
    shooter.update(0.1f, critters);
    std::cout << "After one update (fires at the closest critter):"
              << std::endl;
    printCritters(critters);
    check(nearCritter.getHitPoints() == 20 && farCritter.getHitPoints() == 30 &&
              outOfRange.getHitPoints() == 30,
          "the tower shoots only the closest critter in range");
    shooter.update(0.1f, critters);
    std::cout << "After another 0.1s (still reloading, nothing changes):"
              << std::endl;
    printCritters(critters);
    check(nearCritter.getHitPoints() == 20,
          "the tower does not fire again while reloading");
    shooter.update(1.0f, critters);
    std::cout << "After 1.0s more (reloaded, fires again):" << std::endl;
    printCritters(critters);
    shooter.update(1.0f, critters);
    std::cout << "After 1.0s more (closest critter killed):" << std::endl;
    printCritters(critters);
    check(!nearCritter.isAlive() && nearCritter.getHitPoints() == 0,
          "repeated shots kill the critter at zero hit points");
    std::cout << std::endl;

    // SplashTower damages the target and every critter near it.
    Critter a(5, 30, 1, 1.0, 1, {1, 1});
    Critter b(5, 30, 1, 1.0, 1, {2, 1});
    Critter c(5, 30, 1, 1.0, 1, {5, 1});
    std::vector<Critter *> group = {&a, &b, &c};
    SplashTower splash(0.0f, 1.0f);
    printTower(splash);
    std::cout << "Before firing:" << std::endl;
    printCritters(group);
    splash.update(0.1f, group);
    std::cout << "After firing (target and its neighbour are hit):"
              << std::endl;
    printCritters(group);
    check(a.getHitPoints() == 22 && b.getHitPoints() == 22 &&
              c.getHitPoints() == 30,
          "the Splash Tower also damages critters near its target");
    std::cout << std::endl;

    // HunterTower deals double damage to critters of level 3 or higher.
    Critter weak(5, 50, 1, 1.0, 1, {1, 0});
    Critter strong(5, 50, 1, 1.0, 3, {1, 0});
    std::vector<Critter *> weakList = {&weak};
    std::vector<Critter *> strongList = {&strong};
    HunterTower hunterA(0.0f, 0.0f);
    HunterTower hunterB(0.0f, 0.0f);
    printTower(hunterA);
    hunterA.update(0.1f, weakList);
    hunterB.update(0.1f, strongList);
    std::cout << "Level 1 critter after one shot:" << std::endl;
    printCritters(weakList);
    std::cout << "Level 3 critter after one shot (bonus damage):" << std::endl;
    printCritters(strongList);
    check(weak.getHitPoints() == 42 && strong.getHitPoints() == 34,
          "the Hunter Tower deals double damage to level 3+ critters");
    std::cout << std::endl;
}



void testTowerObserver()
{
    std::cout << "========== Tower observer tests ==========" << std::endl;

    
    class CountingObserver : public Observer
    {
    public:
        int count;

        CountingObserver() : count(0) {}

        void update() override
        {
            ++count;
        }
    };

    DirectTower tower(0.0f, 0.0f);
    CountingObserver counter;
    tower.attach(&counter);

    tower.upgrade();
    check(counter.count == 1, "upgrading notifies the observer once");

    tower.setPosition(2.0f, 1.0f);
    check(counter.count == 2 && tower.getX() == 2.0f && tower.getY() == 1.0f,
          "setPosition moves the tower and notifies the observer");

    Critter target(2, 100, 1, 1.0, 1, Position{3, 1});
    std::vector<Critter *> inRange = {&target};
    tower.update(1.0f, inRange);
    check(counter.count == 3 && target.getHitPoints() < 100,
          "firing at a critter notifies the observer");

    tower.update(0.0f, inRange);
    check(counter.count == 3, "no notify while the tower is reloading");

    std::vector<Critter *> nobody;
    tower.update(1.0f, nobody);
    check(counter.count == 3, "no notify when there is nothing to shoot");

    tower.upgrade();
    tower.upgrade(); // level 4, the max for a Direct Tower
    int before = counter.count;
    check(!tower.upgrade() && counter.count == before,
          "a refused upgrade at max level does not notify");

    CountingObserver second;
    tower.attach(&second);
    tower.setPosition(0.0f, 0.0f);
    check(counter.count == before + 1 && second.count == 1,
          "every attached observer is notified");

    tower.detach(&counter);
    tower.setPosition(1.0f, 1.0f);
    check(counter.count == before + 1 && second.count == 2,
          "a detached observer is not notified anymore");

    TowerObserver view(tower);
    tower.attach(&view);
    tower.setPosition(2.0f, 2.0f);
    check(view.getUpdateCount() == 1, "TowerObserver receives the tower's notify");

    tower.detach(&second);
    tower.detach(&view);
    std::cout << std::endl;
}

namespace
{
void printCritter(std::size_t index, const Critter &critter)
{
    std::cout << "  Critter " << index << ": pos (" << critter.getPosition().x
              << ", " << critter.getPosition().y << ") HP "
              << critter.getHitPoints() << " str " << critter.getStrength()
              << " speed " << critter.getSpeed() << " reward "
              << critter.getReward() << " level " << critter.getLevel();
    if (critter.hasReachedExit())
    {
        std::cout << " [reached exit]";
    }
    else if (!critter.isAlive())
    {
        std::cout << " [killed]";
    }
    std::cout << std::endl;
}

void printSpawned(const CritterGroup &wave)
{
    for (std::size_t i = 0; i < wave.getNextCritterIndex(); ++i)
    {
        printCritter(i, wave.getCritters()[i]);
    }
}
}

void testCritter()
{
    std::cout << "========== Part 3: Critters ==========" << std::endl;

    Map map(6, 3);
    map.setTile(0, 1, TileType::ENTRY);
    map.setTile(1, 1, TileType::PATH);
    map.setTile(2, 1, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(3, 0, TileType::PATH);
    map.setTile(4, 0, TileType::PATH);
    map.setTile(4, 1, TileType::PATH);
    map.setTile(5, 1, TileType::EXIT);
    check(map.validate(), "the critter test map is valid");
    std::cout << "Map used by the critters:" << std::endl;
    map.display();
    std::cout << std::endl;

    // Waves get harder as the level goes up.
    CritterGroupGenerator generator;
    for (int level = 1; level <= 4; ++level)
    {
        CritterGroup wave = generator.generate(level);
        const Critter &sample = wave.getCritters().front();
        std::cout << "Wave " << level << ": " << wave.size()
                  << " critters, HP " << sample.getHitPoints() << ", strength "
                  << sample.getStrength() << ", speed " << sample.getSpeed()
                  << ", reward " << sample.getReward() << std::endl;
    }
    std::cout << std::endl;

    CritterGroup wave1 = generator.generate(1);
    CritterGroup wave4 = generator.generate(4);
    const Critter &easy = wave1.getCritters().front();
    const Critter &hard = wave4.getCritters().front();
    check(wave4.size() > wave1.size() && hard.getHitPoints() > easy.getHitPoints() &&
              hard.getStrength() >= easy.getStrength() &&
              hard.getSpeed() > easy.getSpeed(),
          "later waves have more, tougher and faster critters");
    check(easy.getReward() == 2 * easy.getLevel() &&
              hard.getReward() == 2 * hard.getLevel(),
          "the kill reward is proportional to the critter level");
    std::cout << std::endl;

    // Play wave 1: spawn a critter each turn, move them and shoot the leader.
    CritterGroup wave = generator.generate(1);
    int coins = 20;
    std::cout << "Playing wave 1 with " << wave.size()
              << " critters, starting coins: " << coins << std::endl;

    for (int turn = 1; turn <= 12; ++turn)
    {
        wave.moveCritters(map);
        wave.spawnNext(map);

        // Every other turn, a tower deals 5 damage to the leading critter.
        if (turn % 2 == 0)
        {
            Critter *target = wave.getNextCritter();
            if (target != nullptr)
            {
                std::size_t index =
                    static_cast<std::size_t>(target - &wave.getCritters()[0]);
                int reward = wave.attack(index, 5);
                coins += reward;
                std::cout << "Turn " << turn << ": critter " << index
                          << " hit for 5 damage, reward " << reward
                          << std::endl;
            }
        }

        int stolen = wave.collectExitLoss();
        coins -= stolen;
        if (stolen > 0)
        {
            std::cout << "Turn " << turn << ": critters reached the exit and stole "
                      << stolen << " coins" << std::endl;
        }

        std::cout << "After turn " << turn << " (coins: " << coins
                  << "):" << std::endl;
        printSpawned(wave);
    }
    std::cout << std::endl;

    std::cout << "Critter checks:" << std::endl;
    CritterGroup group = generator.generate(1);
    Critter *first = group.spawnNext(map);
    check(first != nullptr && first->getPosition().x == map.getEntryX() &&
              first->getPosition().y == map.getEntryY(),
          "critters spawn on the map entry");
    check(group.getNextCritterIndex() == 1,
          "critters in a group spawn one after the other");

    first->move(map);
    check(first->getPosition().x == 1 && first->getPosition().y == 1,
          "a critter steps along the path toward the exit");
    for (int step = 0; step < 6; ++step)
    {
        first->move(map);
    }
    check(first->hasReachedExit(),
          "a critter following the shortest path reaches the exit");
    check(group.collectExitLoss() == first->getStrength() &&
              group.collectExitLoss() == 0,
          "a critter at the exit steals coins equal to its strength, once");

    Critter *second = group.spawnNext(map);
    const std::size_t secondIndex = group.getNextCritterIndex() - 1;
    const int fullHitPoints = second->getHitPoints();
    check(group.attack(secondIndex, 5) == 0 &&
              second->getHitPoints() == fullHitPoints - 5,
          "an attack reduces hit points without a reward while alive");
    check(group.attack(secondIndex, fullHitPoints) == second->getReward() &&
              !second->isAlive(),
          "killing a critter pays its reward");
    check(group.attack(secondIndex, 5) == 0,
          "a dead critter does not pay its reward twice");
    std::cout << std::endl;
}

// Checks that notify() reaches an attached observer when the wave changes.
void testCritterObserver()
{
    std::cout << "========== Critter observer tests ==========" << std::endl;

    class CountingObserver : public Observer
    {
    public:
        int count;

        CountingObserver() : count(0) {}

        void update() override
        {
            ++count;
        }
    };

    Map map(4, 2);
    map.setTile(0, 0, TileType::ENTRY);
    map.setTile(1, 0, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(3, 0, TileType::EXIT);

    CritterGroup group;
    CountingObserver counter;
    group.attach(&counter);

    Critter sample(4, 20, 2, 1.0, 1);
    group.addCritter(sample);
    check(counter.count == 1, "adding a critter notifies the observer once");

    group.spawnNext(map);
    check(counter.count == 2, "spawnNext notifies the observer");

    group.removeCritter(0);
    check(counter.count == 3 && group.empty(),
          "removeCritter notifies the observer and clears the wave");

    group.detach(&counter);
    std::cout << std::endl;
}

int main()
{
    std::cout << "################################################" << std::endl;
    std::cout << "#  Tower Defense: running tests                #" << std::endl;
    std::cout << "#  The GUI will start after the tests are done #" << std::endl;
    std::cout << "################################################" << std::endl
              << std::endl;

    testMap();
    testTower();
    testTowerObserver();
    testCritter();
    testCritterObserver();

    std::cout << "========== Test summary ==========" << std::endl;
    std::cout << passedChecks << " passed, " << failedChecks << " failed"
              << std::endl;
    std::cout << (failedChecks == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED")
              << std::endl
              << std::endl;

    runTowerObserverDemo();
    runCritterObserverDemo();

    std::cout << "========== Starting the GUI ==========" << std::endl;
    std::cout << "Close the game window to exit." << std::endl;
    runGui();

    return failedChecks == 0 ? 0 : 1;
}
