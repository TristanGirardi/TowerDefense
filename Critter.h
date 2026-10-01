#ifndef CRITTER_H
#define CRITTER_H

#include "Map.h"

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

class Critter
{
private:
    int reward;
    int hitPoints;
    int strength;
    double speed;
    int level;
    Position position;
    double movementRemainder;
    bool exitReached;
    bool rewardClaimed;
    bool exitLossClaimed;

    Position nextPosition(const Map &map) const;

public:
    Critter(int reward, int hitPoints, int strength, double speed, int level);
    Critter(int reward, int hitPoints, int strength, double speed, int level,
            Position startingPosition);

    int getReward() const;
    int getHitPoints() const;
    int getStrength() const;
    double getSpeed() const;
    int getLevel() const;
    Position getPosition() const;
    bool isAlive() const;
    bool hasReachedExit() const;

    void takeDamage(int damage);

    // Moves by speed map cells. A fractional speed is accumulated between turns.
    void move(const Map &map);
    void move(const Map &map, double elapsedSeconds);

    // Returns the reward once, when this critter has been killed.
    int claimReward();

    // Returns the amount of coins stolen when the exit is reached.
    int coinsStolenAtExit() const;
    int claimCoinsStolenAtExit();
};

#endif
