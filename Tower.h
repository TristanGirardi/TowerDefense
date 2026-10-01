#ifndef TOWER_H
#define TOWER_H

#include <string>
#include <vector>

class Critter;

struct TowerStats {
    int   cost;      // Buy cost (level 1) or upgrade cost (levels 2+).
    float range;     // Detection range, in grid cells.
    int   power;     // Damage dealt per shot.
    float fireRate;  // Shots per second.
}; 

class Tower {
public:
    
    Tower(const std::string& name, float x, float y,
          const std::vector<TowerStats>& levelTable);
 
    virtual ~Tower() = default;
 
    
    //Cost to buy this tower (level 1 cost).
    int getBuyCost() const;
 
    //true if the tower is not at its maximum level.
    bool canUpgrade() const;
 
    //Cost of the next upgrade, or -1 if already at max level.
    int getUpgradeCost() const;
 
    
     // true if upgraded, false if already at max level.
    bool upgrade();
 
    //Coins refunded if sold now (50% of total spent).
    int getRefundValue() const;
 
    
 
   
     //Reduces the fire cooldown; when ready, selects a target in rangeand fires at it.
    
    void update(float dt, std::vector<Critter*>& critters);
 
    
 
    const std::string& getName() const;
    int   getLevel() const;     ///< Current level, starting at 1.
    int   getMaxLevel() const;
    float getX() const;
    float getY() const;
    const TowerStats& getStats() const; ///< Stats at the current level.
 
protected:
    ///true if the critter is alive and within range.
    bool isInRange(const Critter& critter) const;
 
    //The closest critter in range, or nullptr if none.
    Critter* selectTarget(const std::vector<Critter*>& critters) const;
 
    /// return Euclidean distance between two points.
    static float distance(float x1, float y1, float x2, float y2);
 
    
     //What a shot does. Each tower type defines its own effect.
     //  target  = The selected critter.
     // critters = All critters (needed by area-of-effect towers).
     
    virtual void applyEffect(Critter& target,
                             std::vector<Critter*>& critters) = 0;
 
private:
    std::string name;
    float x;
    float y;
    std::vector<TowerStats> levels;
    int   level;     ///< Index into levels (0-based).
    float cooldown;  ///< Seconds until the tower can fire again.
};
 

//Basic tower: deals damage to a single target.

class DirectTower : public Tower {
public:
    DirectTower(float x, float y);
 
protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;
};
 

//Area-of-effect tower: damages the target and nearby critters.
 
class SplashTower : public Tower {
public:
    SplashTower(float x, float y);
 
protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;
 
private:
    float splashRadius; ///< Radius around the target that is also hit.
};
 

//Hunter tower: deals bonus damage to high-level critters.
class HunterTower : public Tower {
public:
    HunterTower(float x, float y);

protected:
    void applyEffect(Critter& target,
                     std::vector<Critter*>& critters) override;

private:
    int   bonusMinLevel;    ///< Critters at or above this level take bonus damage.
    float bonusMultiplier;  ///< Damage multiplier against them (2.0 = double).
};
















#endif