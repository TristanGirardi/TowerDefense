#ifndef TOWER_H
#define TOWER_H

#include <string>
#include <vector>

class Critter;

// Stats for one level of a tower. Each tower has a list of these, one per level.
struct TowerStats {
    int   cost;      // buy price at level 1, upgrade price after that
    float range;     // in grid cells
    int   power;     // damage per shot
    float fireRate;  // shots per second
};

// Base class for all towers. Buying, upgrading, selling, aiming and reloading
// work the same for every tower, so they live here. Only what a shot does
// changes, so that's the one virtual function, applyEffect().
class Tower {
public:
    Tower(const std::string& name, float x, float y,
          const std::vector<TowerStats>& levelTable);

    virtual ~Tower() = default;

    int getBuyCost() const;

    // true if the tower is not at its maximum level.
    bool canUpgrade() const;
    int getUpgradeCost() const;   // -1 at max level
    bool upgrade();               // false at max level; the caller pays for it
    int getRefundValue() const;   // half of everything spent on it

    // Called every frame: reload, find the closest critter in range, shoot.
    void update(float dt, std::vector<Critter*>& critters);

    const std::string& getName() const;
    int   getLevel() const;     // starts at 1
    int   getMaxLevel() const;
    float getX() const;
    float getY() const;
    const TowerStats& getStats() const; // stats for the current level

protected:
    // true if the critter is alive and within range.
    bool isInRange(const Critter& critter) const;

    // The closest critter in range, or nullptr if none.
    Critter* selectTarget(const std::vector<Critter*>& critters) const;

    static float distance(float x1, float y1, float x2, float y2);

    // What a shot does. Each tower type defines its own effect.
    // target = the selected critter
    // critters = all critters (needed by area-of-effect towers)
    virtual void applyEffect(Critter& target,
                             std::vector<Critter*>& critters) = 0;

private:
    std::string name;
    float x;
    float y;
    std::vector<TowerStats> levels;
    int   level;     // 0-based index into levels
    float cooldown;  // seconds until it can shoot again
};

// Direct damage: hits one critter.
class DirectTower : public Tower {
public:
    DirectTower(float x, float y);

protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;
};

// Area damage: also hits critters close to the target.
class SplashTower : public Tower {
public:
    SplashTower(float x, float y);

protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;

private:
    float splashRadius; // in cells, around the target
};

// Hunter tower: deals bonus damage to high-level critters.
class HunterTower : public Tower {
public:
    HunterTower(float x, float y);

protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;

private:
    int   bonusMinLevel;    // Critters at or above this level take bonus damage.
    float bonusMultiplier;  // Damage multiplier against them (2.0 = double).
};

#endif
