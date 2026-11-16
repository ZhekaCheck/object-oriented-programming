#pragma once

enum class UnitType {
    Swordsman, Spearman,
    LightArcher, HeavyArcher,
    Cataphract, HorseArcher
};

inline const char* toString(UnitType type) {
    switch (type) {
    case UnitType::Swordsman: return "Swordsman";
    case UnitType::Spearman: return "Spearman";
    case UnitType::LightArcher: return "LightArcher";
    case UnitType::HeavyArcher: return "HeavyArcher";
    case UnitType::Cataphract: return "Cataphract";
    case UnitType::HorseArcher: return "HorseArcher";
    }
    return "Unknown";
}