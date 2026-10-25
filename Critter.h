
#ifndef CRITTER_H
#define CRITTER_H

#include "Map.h"
#include "Observable.h" // Observer pattern support

// A cell on the map. Critters are always on exactly one cell.
struct Position
{
    int x;
    int y;

    bool operator==(const Position &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position &other) const
    {
        return !(*this == other);
    }
};

// One enemy walking from the entry to the exit.
//   reward    - coins you get for killing it
//   hitPoints - it dies at 0
//   strength  - coins it steals if it reaches the exit
//   speed     - cells per turn (or per second in the GUI)
//   level     - the wave it came from
// It has no fixed route: each move it finds the shortest way to the exit
// and takes one step.
class Critter : public Observable // Critter is now Observable
{
private:
    int reward;
    int hitPoints;
    int strength;
    double speed;
    int level;
    Position position;

    double movementRemainder; // part of a step not walked yet

    bool exitReached;
    bool rewardClaimed;
    bool exitLossClaimed;

    // Next cell toward the exit, or the current one if it can't move.
    Position nextPosition(const Map &map) const;

public:
    // Starts at (0, 0); CritterGroup puts it on the entry when it spawns.
    Critter(int reward, int hitPoints, int strength, double speed, int level);
    Critter(int reward, int hitPoints, int strength, double speed, int level,
            Position startingPosition);

    int getReward() const;
    int getHitPoints() const;
    int getStrength() const;
    double getSpeed() const;
    int getLevel() const;
    Position getPosition() const;

    // False once it's dead or has escaped, so towers stop shooting it.
    bool isAlive() const;
    bool hasReachedExit() const;

    void takeDamage(int damage);

    void move(const Map &map);                        // one turn
    void move(const Map &map, double elapsedSeconds); // used by the GUI

    // These two pay out only once, so nothing gets counted twice.
    int claimReward();            // the reward, once it's dead
    int claimCoinsStolenAtExit(); // its strength, once it's escaped
};

#endif
