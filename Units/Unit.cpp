#include "Unit.h"

Unit::Unit(const Health& health, const Armor& armor, const Attack& attack, int x, int y)
    : health_(health), armor_(armor), attack_(attack), x_(x), y_(y) {}

bool Unit::moveTo(int newX, int newY, int fieldWidth, int fieldHeight) {
    if (newX < 0 || newY < 0 || newX >= fieldWidth || newY >= fieldHeight)
        return false;
    x_ = newX;
    y_ = newY;
    return true;
}

void Unit::takeDamage(int rawDamage) {
    health_.decrease(armor_.reduceDamage(rawDamage));
}

bool Unit::isAlive() const { return !health_.isZero(); }

int Unit::calcDamageTo(const Unit& target) const {
    return attack_.calculateDamage(target.armor_);
}
