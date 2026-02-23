/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <variant>
    #include <string>
    #include <unordered_map>

namespace dng {

using AttributeValue = std::variant<int32_t, float, std::string>;

struct Attribute {
    std::string name;
    AttributeValue value;
    AttributeValue max_value;

    std::unordered_map<std::string, std::string> metadata;
};

}  // namespace dng
