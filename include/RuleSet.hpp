/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <string>
    #include <unordered_map>
    #include <iostream>
    #include <vector>

    #include <json/json.h>
    #include <fstream>

    #include "Stat.hpp"
    #include "Attribute.hpp"
    #include "Item.hpp"
    #include "Entity.hpp"

namespace dng {

struct RuleSet {
    RuleSet() = delete;
    static inline std::string name = "";
    static inline std::unordered_map<std::string, Stat> stat_definitions = {};
    static inline std::vector<Attribute> attribute_definitions = {};
    static inline std::unordered_map<std::string, std::string> global_rules = {};
    static inline std::vector<Item> item_templates = {};
    static inline std::vector<Entity> entities = {};

    static void init(const std::string& path) {
        Json::Value raw;
        std::ifstream file(path, std::ifstream::binary);
        file >> raw;

        auto& stats = raw["stats"];
        for (auto& stat : stats) {
            stat_definitions.insert_or_assign(stat["name"].asString(),
                Stat(stat["name"].asString(),
                    stat["base"].asUInt(),
                    stat["modifier"].asString()));
        }
        auto& characts = raw["characters"];
        for (auto& charact : characts) {
            entities.emplace_back(Entity(charact));
        }
    }
};

}  // namespace dng
