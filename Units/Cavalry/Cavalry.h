#pragma once
#include "Unit.h"

class Cavalry : public Unit {
public:
    Cavalry(const Health& h, const Armor& a, const Attack& at, int x, int y, int speed)
        : Unit(h, a, at, x, y), speed_(speed) {
    }
    int speed() const { return speed_; }
private:
    int speed_;
};

class Cataphract : public Cavalry {
public:
    Cataphract(int x, int y) : Cavalry(Health(150), Armor(8), Attack(20), x, y, 3) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Cataphract>(*this); }
    UnitType getType() const override { return UnitType::Cataphract; }
};

class HorseArcher : public Cavalry {
public:
    HorseArcher(int x, int y) : Cavalry(Health(90), Armor(2), Attack(16), x, y, 5) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<HorseArcher>(*this); }
    UnitType getType() const override { return UnitType::HorseArcher; }
};