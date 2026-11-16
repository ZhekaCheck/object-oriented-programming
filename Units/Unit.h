#pragma once
#include <memory>
#include "Health.h"
#include "Armor.h"
#include "Attack.h"
#include "UnitType.h"

class Unit {
public:
    Unit(const Health& health, const Armor& armor, const Attack& attack, int x, int y);
    virtual ~Unit() = default;

    Unit(const Unit&) = default;
    Unit& operator=(const Unit&) = default;

    virtual std::unique_ptr<Unit> clone() const = 0;
    virtual UnitType getType() const = 0;

    const Health& health() const { return health_; }
    const Armor& armor()  const { return armor_; }
    const Attack& attack() const { return attack_; }
    int x() const { return x_; }
    int y() const { return y_; }

    bool moveTo(int newX, int newY, int fieldWidth, int fieldHeight);
    void takeDamage(int rawDamage);
    bool isAlive() const;
    int calcDamageTo(const Unit& target) const;

protected:
    Health health_;
    Armor  armor_;
    Attack attack_;
    int x_;
    int y_;
};