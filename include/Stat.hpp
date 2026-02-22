/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <ostream>
    #include <string>
    #include <unordered_map>

namespace dng {

class Stat {
  public:
    Stat() = default;
    Stat(uint16_t base, const std::string& modif);

    int32_t roll(int32_t value) const;
    int32_t applyModifier(int32_t value) const;

  private:
    uint16_t _base;
    std::string _modifier;
};

class StatManager {
  public:
    static void createStat(const std::string &name, int32_t base,
        const std::string &modif) {
        _stats.insert_or_assign(name, Stat(base, modif));
    }

    static int32_t roll(const std::string& name, int32_t value) {
        return _stats.at(name).roll(value);
    }

  private:
    static inline std::unordered_map<std::string, Stat> _stats = {};
};

typedef std::unordered_map<std::string, int32_t> StatPool;

}  // namespace dng

std::ostream& operator<<(std::ostream &os, const dng::StatPool &spool);
