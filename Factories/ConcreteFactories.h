#pragma once
#include <stdexcept>
#include "UnitFactory.h"
#include "Infantry.h"
#include "Archer.h"
#include "Cavalry.h"

class InfantryFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(UnitType type, int x, int y) const override {
        switch (type) {
        case UnitType::Swordsman: return std::make_unique<Swordsman>(x, y);
        case UnitType::Spearman: return std::make_unique<Spearman>(x, y);
        default:
            throw std::invalid_argument("InfantryFactory: это не пехота");
        }
    }
};

class ArcherFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(UnitType type, int x, int y) const override {
        switch (type) {
        case UnitType::LightArcher: return std::make_unique<LightArcher>(x, y);
        case UnitType::HeavyArcher: return std::make_unique<HeavyArcher>(x, y);
        default:
            throw std::invalid_argument("ArcherFactory: это не лучник");
        }
    }
};

class CavalryFactory : public UnitFactory {
public:
    std::unique_ptr<Unit> createUnit(UnitType type, int x, int y) const override {
        switch (type) {
        case UnitType::Cataphract: return std::make_unique<Cataphract>(x, y);
        case UnitType::HorseArcher: return std::make_unique<HorseArcher>(x, y);
        default:
            throw std::invalid_argument("CavalryFactory: это не конница");
        }
    }
};