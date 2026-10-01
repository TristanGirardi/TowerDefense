#include "Tower.h"
#include "Critter.h"

#include <stdexcept>
#include <cmath>

//Constructor

Tower::Tower(const std::string& name, float x,float y, const std::vector<TowerStats>& levelTable){
    this->name = name;
    this->x = x;
    this->y = y;
    levels = levelTable;
    level = 0;
    cooldown = 0.0f;
}


//Economy: Costs for buying, upgrading, refund value

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
    return true;
}


int Tower::getRefundValue() const{
    int spent =0;
    for (int i=0; i <=level;i++){
        spent += levels[i].cost;//buy cost + every upgrade paid
    }
    return spent/2;//The refund will be half what is spent
}

//Getters
const std::string& Tower::getName()const {return name;}
int Tower::getLevel() const {return level+1;}
int Tower::getMaxLevel() const {return static_cast<int>(levels.size());}
float Tower::getX() const {return x;}
float Tower::getY() const {return y;}
const TowerStats& Tower::getStats() const {return levels[level];}



//Combat Helpers

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


Critter* Tower::selectTarget(const std::vector<Critter*>& critters) const {
    Critter* best = nullptr;
    float bestDistance = 0.0f;

    for (Critter* c : critters){
        if (c == nullptr || !isInRange(*c)){
            continue; //skip those out of range or dead
        }

        float d = distance(x,y,c->getPosition().x,c->getPosition().y);

        if (best == nullptr || d < bestDistance){
            best = c;
            bestDistance = d;
        }
    }
    return best; //nullptr if nothing in range
}

//Detect->Select->Fire

void Tower::update(float dt, std::vector<Critter*>& critters){
    if (cooldown > 0.0f){
        cooldown -= dt;
    }

    if (cooldown > 0.0f){
        return;//Still reload
    }

    Critter* target = selectTarget(critters);
    if (target == nullptr){
        cooldown = 0.0f;//stay ready, shoot when soemthing is in range
        return ;
    }

    applyEffect(*target,critters);//Fires, effect depends on tower

    cooldown = 1.0f/getStats().fireRate;
}

//Tower #1: Direct Tower(Default)
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
 

//Tower #2: SplashTower: damages the target and every critter near it

 
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
 
    // Hit every other living critter close to the target.
    for (Critter* c : critters) {
        if (c == nullptr || c == &target || !c->isAlive()) {
            continue;  // the target itself is hit separately below
        }
        float d = distance(target.getPosition().x, target.getPosition().y, c->getPosition().x, c->getPosition().y);
        if (d <= splashRadius) {
            c->takeDamage(power);
        }
    }
 
    target.takeDamage(power);
}
 

//Tower #3: HunterTower: bonus damage against high-level critters

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
 

