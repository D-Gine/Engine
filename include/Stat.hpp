/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <cstdint>
    #include <string>

namespace dng {

struct Stat {
    std::string name;
    uint16_t base;
    std::string modifier;

    Stat(const std::string& name, uint16_t base, const std::string& modif);
    int32_t applyModifier(int32_t val) const;
};

}  // namespace dng
