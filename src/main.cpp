/*
 * Copyright 2026 <D&Gine Group>
 */

#include <print>
#include <iostream>

#include <RuleSet.hpp>
#include <Entity.hpp>

int main() {
    srand(time(0));
    dng::RuleSet::init("tests/ruleset.json");

    std::string input;
    while (1) {
        std::cin >> input;
        auto res = dng::RuleSet::entities[0].roll(input);
        if (res)
            std::println("You have rolled {} + {} ({}) in {}.",
                (*res).first, (*res).second, (*res).first + (*res).second, input);
        else
            std::println("Error: {}", res.error());
    }
    return 0;
}
