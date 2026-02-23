/*
 * Copyright 2026 <D&Gine Group>
 */

#include "Stat.hpp"

namespace dng {

Stat::Stat(const std::string& name, uint16_t base, const std::string& modif)
    : name(name), base(base), modifier(modif) {}

int32_t Stat::applyModifier(int32_t value) const {
    // Forced for now
    return (value - 10) / 2;
}

}  // namespace dng
