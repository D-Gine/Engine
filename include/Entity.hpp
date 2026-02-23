/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <cstdint>
    #include <json/value.h>
    #include <string>
    #include <expected>
    #include <unordered_map>

    #include <Stat.hpp>
    #include <Item.hpp>
    #include <Attribute.hpp>

namespace dng {

struct Entity {
  public:
    Entity(const Json::Value& val);
    std::expected<std::pair<int32_t, int32_t>, std::string> roll(
        const std::string& sname);
    void action(const std::string& name);

  private:
    std::string name;
    std::unordered_map<std::string, int32_t> _stats;
};

}  // namespace dng
