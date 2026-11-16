#pragma once
#include "Unit.h"

class Infantry : public Unit {
public:
    Infantry(const Health& h, const Armor& a, const Attack& at, int x, int y)
        : Unit(h, a, at, x, y) {
    }
};

class Swordsman : public Infantry {
public:
    Swordsman(int x, int y) : Infantry(Health(100), Armor(5), Attack(15), x, y) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Swordsman>(*this); }
    UnitType getType() const override { return UnitType::Swordsman; }
};

class Spearman : public Infantry {
public:
    Spearman(int x, int y) : Infantry(Health(90), Armor(4), Attack(12), x, y) {}
    std::unique_ptr<Unit> clone() const override { return std::make_unique<Spearman>(*this); }
    UnitType getType() const override { return UnitType::Spearman; }
};