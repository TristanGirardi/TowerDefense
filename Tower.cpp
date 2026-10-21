#include "Tower.h"
#include "Critter.h"


#include <stdexcept>
#include <cmath>

// A new tower starts at level 1 and can shoot right away.
Tower::Tower(const std::string& name, float x,float y, const std::vector<TowerStats>& levelTable){
    this->name = name;
    this->x = x;
    this->y = y;
    levels = levelTable;
    level = 0;
    cooldown = 0.0f;
}


// Buying, upgrading and selling

int Tower::getBuyCost() const{
    return levels[0].cost;
}

bool Tower::canUpgrade() const{
    return level+1 < static_cast<int>(levels.size());
}



int Tower::getUpgradeCost() const{
    if (!canUpgrade()){
        return -1;
    }
    return levels[level+1].cost;
}


bool Tower::upgrade() {
    if (!canUpgrade()){
        return false;
    }
    level++;
    notify();
    return true;
}


int Tower::getRefundValue() const{
    int spent =0;
    for (int i=0; i <=level;i++){
        spent += levels[i].cost; // buy price + every upgrade
    }
    return spent/2; // you get half of it back
}

// Getters
const std::string& Tower::getName()const {return name;}
int Tower::getLevel() const {return level+1;}
int Tower::getMaxLevel() const {return static_cast<int>(levels.size());}
float Tower::getX() const {return x;}
float Tower::getY() const {return y;}
const TowerStats& Tower::getStats() const {return levels[level];}

//Setters
void Tower::setPosition(float x, float y){
    this->x = x;
    this->y = y;
    notify(); // tell observers the tower moved
}

// Targeting

float Tower::distance(float x1, float y1, float x2, float y2){
    float dx = x2-x1;
    float dy = y2-y1;
    return std::sqrt((dx*dx)+(dy*dy));
}


bool Tower::isInRange(const Critter& critter) const{
    if(!critter.isAlive()){
        return false;
    }

    return distance(x,y,critter.getPosition().x,critter.getPosition().y) <=getStats().range;
}


// We go for the closest critter. On a tie, the first one in the list wins.
Critter* Tower::selectTarget(const std::vector<Critter*>& critters) const {
    Critter* best = nullptr;
    float bestDistance = 0.0f;

    for (Critter* c : critters){
        if (c == nullptr || !isInRange(*c)){
            continue; // dead, already escaped, or too far away
        }

        float d = distance(x,y,c->getPosition().x,c->getPosition().y);

        if (best == nullptr || d < bestDistance){
            best = c;
            bestDistance = d;
        }
    }
    return best; // still nullptr if nobody was in range
}

// Detect -> select -> shoot, once per frame

void Tower::update(float dt, std::vector<Critter*>& critters){
    if (cooldown > 0.0f){
        cooldown -= dt;
    }

    if (cooldown > 0.0f){
        return; // still reloading
    }

    Critter* target = selectTarget(critters);
    if (target == nullptr){
        // stay loaded so it fires as soon as something walks into range
        cooldown = 0.0f;
        return ;
    }

    applyEffect(*target,critters);//Fires, effect depends on tower

    cooldown = 1.0f/getStats().fireRate; // e.g. 2 shots/s = wait 0.5s

    notify();
}

// Tower types. Each one only has a name, an upgrade table and applyEffect().

//Tower #1
// Direct Tower: cheapest, and it has the most levels.
DirectTower::DirectTower(float x, float y)
    : Tower("Direct Tower", x, y, {
        //  cost  range  power  fireRate
        {   100,  3.0f,   10,    1.5f },  // level 1
        {    60,  3.5f,   15,    1.8f },  // level 2
        {    90,  4.0f,   22,    2.2f },  // level 3
        {   120,  4.5f,   28,    2.6f },  // level 4
      })
{}
 
void DirectTower::applyEffect(Critter& target,std::vector<Critter*>& /*critters*/) {
    target.takeDamage(getStats().power);
}

//Tower #2
// Splash Tower: weaker and slower, but it hits a whole group at once.
SplashTower::SplashTower(float x, float y)
    : Tower("Splash Tower", x, y, {
        //  cost  range  power  fireRate
        {   150,  2.5f,    8,    0.8f },  // level 1
        {   100,  3.0f,   12,    0.9f },  // level 2
        {   140,  3.5f,   16,    1.0f },  // level 3
      }),
      splashRadius(1.5f)
{}
 
void SplashTower::applyEffect(Critter& target,
                              std::vector<Critter*>& critters) {
    int power = getStats().power;
 
    // The blast is centred on the target, not on the tower.
    for (Critter* c : critters) {
        if (c == nullptr || c == &target || !c->isAlive()) {
            continue;  // the target gets hit on its own below
        }
        float d = distance(target.getPosition().x, target.getPosition().y, c->getPosition().x, c->getPosition().y);
        if (d <= splashRadius) {
            c->takeDamage(power);
        }
    }
 
    target.takeDamage(power);
}

//Tower #3
// Hunter Tower: does double damage to critters of level 3 and up.
HunterTower::HunterTower(float x, float y)
    : Tower("Hunter Tower", x, y, {
        //  cost  range  power  fireRate
        {   120,  3.5f,    8,    1.0f },  // level 1
        {    80,  4.0f,   12,    1.1f },  // level 2
        {   130,  4.5f,   16,    1.3f },  // level 3
      })
{
    bonusMinLevel = 3;
    bonusMultiplier = 2.0f;
}

void HunterTower::applyEffect(Critter& target, std::vector<Critter*>& /*critters*/) {
    int damage = getStats().power;

    if (target.getLevel() >= bonusMinLevel) {
        damage = static_cast<int>(damage * bonusMultiplier);
    }

    target.takeDamage(damage);
}
 

