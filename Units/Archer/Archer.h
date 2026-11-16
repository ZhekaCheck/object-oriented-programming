#pragma once
#include "Unit.h"

class Archer : public Unit {
public:
    Archer(const Health& h, const Armor& a, const Attack& at, int x, int y, int range)
        : Unit(h, a, at, x, y), range_(range) {
    }
    int range() const { return range_; }
private:
    int range_;
};

class LightArcher : public Archer {
public:
    LightArcher(int x, int y) : Archer(Health(60), Armor(1), Attack(14), x, y, 6) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<LightArcher>(*this); }
    UnitType getType() const override { return UnitType::LightArcher; }
};

class HeavyArcher : public Archer {
public:
    HeavyArcher(int x, int y) : Archer(Health(80), Armor(3), Attack(18), x, y, 5) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<HeavyArcher>(*this); }
    UnitType getType() const override { return UnitType::HeavyArcher; }
};