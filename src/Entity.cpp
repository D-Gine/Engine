/*
 * Copyright 2026 <D&Gine Group>
 */

#include "RuleSet.hpp"
#include "Entity.hpp"

namespace dng {

Entity::Entity(const Json::Value& raw) : name(raw["name"].asString()) {
    const auto& stat_names = raw["stats"].getMemberNames();
    for (const auto& stat_name : stat_names) {
        _stats.insert_or_assign(stat_name, raw["stats"][stat_name].asInt());
    }
}

std::expected<std::pair<int32_t, int32_t>, std::string> Entity::roll(const std::string& name) {
    if (_stats.find(name) == _stats.end())
        return std::unexpected<std::string>("this entity doesn't have this statistic");
    if (RuleSet::stat_definitions.find(name) == RuleSet::stat_definitions.end())
        return std::unexpected<std::string>("can't find statistic in ruleset");
    int32_t dice = rand() % RuleSet::stat_definitions.at(name).base;

    return std::pair(
        dice,
        RuleSet::stat_definitions.at(name).applyModifier(_stats.at(name)));
}

}  // namespace dng
