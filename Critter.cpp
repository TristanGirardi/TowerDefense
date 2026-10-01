#include "Critter.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <vector>

namespace
{
const Position directions[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

bool canEnter(const Map &map, const Position &position)
{
    if (position.x < 0 || position.x >= map.getWidth() ||
        position.y < 0 || position.y >= map.getHeight())
    {
        return false;
    }

    const TileType tile = map.getTile(position.x, position.y);
    return tile == TileType::PATH || tile == TileType::ENTRY ||
           tile == TileType::EXIT;
}
}

Critter::Critter(int reward, int hitPoints, int strength, double speed, int level)
    : Critter(reward, hitPoints, strength, speed, level,
              {0, 0})
{
}

Critter::Critter(int reward, int hitPoints, int strength, double speed, int level,
                 Position startingPosition)
    : reward(reward),
      hitPoints(hitPoints),
      strength(strength),
      speed(speed),
      level(level),
      position(startingPosition),
      movementRemainder(0.0),
      exitReached(false),
      rewardClaimed(false),
      exitLossClaimed(false)
{
    if (reward < 0 || hitPoints <= 0 || strength < 0 || speed < 0.0 ||
        level <= 0)
    {
        throw std::invalid_argument("Critter statistics are invalid.");
    }
}

int Critter::getReward() const
{
    return reward;
}

int Critter::getHitPoints() const
{
    return hitPoints;
}

int Critter::getStrength() const
{
    return strength;
}

double Critter::getSpeed() const
{
    return speed;
}

int Critter::getLevel() const
{
    return level;
}

Position Critter::getPosition() const
{
    return position;
}

bool Critter::isAlive() const
{
    return hitPoints > 0 && !exitReached;
}

bool Critter::hasReachedExit() const
{
    return exitReached;
}

void Critter::takeDamage(int damage)
{
    if (damage < 0)
    {
        throw std::invalid_argument("Damage cannot be negative.");
    }

    if (isAlive())
    {
        hitPoints = std::max(0, hitPoints - damage);
    }
}

Position Critter::nextPosition(const Map &map) const
{
    if (position.x == map.getExitX() && position.y == map.getExitY())
    {
        return position;
    }

    const Position exit = {map.getExitX(), map.getExitY()};
    if (!canEnter(map, position) || !canEnter(map, exit))
    {
        return position;
    }

    std::vector<std::vector<bool>> visited(
        map.getHeight(), std::vector<bool>(map.getWidth(), false));
    std::vector<std::vector<Position>> previous(
        map.getHeight(),
        std::vector<Position>(map.getWidth(),
                              {std::numeric_limits<int>::min(),
                               std::numeric_limits<int>::min()}));
    std::queue<Position> pending;
    pending.push(position);
    visited[position.y][position.x] = true;

    while (!pending.empty())
    {
        const Position current = pending.front();
        pending.pop();
        if (current == exit)
        {
            break;
        }

        for (const Position &direction : directions)
        {
            const Position candidate = {
                current.x + direction.x, current.y + direction.y};
            if (canEnter(map, candidate) &&
                !visited[candidate.y][candidate.x])
            {
                visited[candidate.y][candidate.x] = true;
                previous[candidate.y][candidate.x] = current;
                pending.push(candidate);
            }
        }
    }

    if (!visited[exit.y][exit.x])
    {
        return position;
    }

    Position step = exit;
    while (previous[step.y][step.x] != position)
    {
        step = previous[step.y][step.x];
    }
    return step;
}

void Critter::move(const Map &map)
{
    move(map, 1.0);
}

void Critter::move(const Map &map, double elapsedSeconds)
{
    if (elapsedSeconds < 0.0)
    {
        throw std::invalid_argument("Elapsed time cannot be negative.");
    }
    if (!isAlive())
    {
        return;
    }

    movementRemainder += speed * elapsedSeconds;
    const int steps = static_cast<int>(std::floor(movementRemainder));
    movementRemainder -= steps;

    for (int step = 0; step < steps && isAlive(); ++step)
    {
        const Position next = nextPosition(map);
        if (next == position)
        {
            break;
        }
        position = next;
        if (position.x == map.getExitX() && position.y == map.getExitY())
        {
            exitReached = true;
        }
    }
}

int Critter::claimReward()
{
    if (isAlive() || rewardClaimed)
    {
        return 0;
    }
    rewardClaimed = true;
    return reward;
}

int Critter::coinsStolenAtExit() const
{
    return exitReached ? strength : 0;
}

int Critter::claimCoinsStolenAtExit()
{
    if (!exitReached || exitLossClaimed)
    {
        return 0;
    }
    exitLossClaimed = true;
    return strength;
}
