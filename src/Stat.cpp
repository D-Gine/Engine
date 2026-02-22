/*
 * Copyright 2026 <D&Gine Group>
 */

#include <cstdint>
#include <print>

#include "Stat.hpp"

namespace dng {

Stat::Stat(uint16_t base, const std::string& modif) :
    _base(base), _modifier(modif)
{}

int32_t Stat::roll(int32_t value) const {
    int32_t dice = rand() % _base;
    int32_t modified = dice + applyModifier(value);

    std::println("Rolled {} - natural {}", modified, dice);
    return modified;
}

int32_t Stat::applyModifier(int32_t value) const {
    // Forced for now
    return (value - 10) / 2;
}

}  // namespace dng

std::ostream& operator<<(std::ostream &os, const dng::StatPool &spool) {
    for (auto& stat : spool)
        os << '\t' << stat.first << " - " << stat.second << '\n';
    return os;
}
