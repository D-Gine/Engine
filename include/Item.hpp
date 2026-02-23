/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <string>
    #include <set>
    #include <unordered_map>
    #include <vector>

namespace dng {

struct Effect {
    std::string trigger;
    std::string expression;
    std::string target;
};

struct Item {
    std::string name;
    std::set<std::string> tags;
    std::unordered_map<std::string, int32_t> stat_modifiers;
    std::vector<Effect> effects;
    std::unordered_map<std::string, std::string> metadata;
};

}  // namespace dng
