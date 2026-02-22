/*
 * Copyright 2026 <D&Gine Group>
 */

#include <cstdint>
#include <expected>
#include <iostream>
#include <print>

#include "Stat.hpp"

class TempPlayer {
  public:
    TempPlayer() :
        _spool({
            {"Force", 18},
            {"Dexterité", 8},
            {"Constitution", 15},
            {"Intelligence", 8},
            {"Sagesse", 6},
            {"Charisme", 12},
        })
    {
        std::println("New player created\nStats:");
        std::cout << _spool << std::endl;
    }

    std::expected<int32_t, std::string> getStat(const std::string& name) {
        if (_spool.find(name) == _spool.end())
            return std::unexpected<std::string>("can't find the stat");
        return _spool.at(name);
    }

  private:
    dng::StatPool _spool;
};

int main() {
    srand(time(0));
    dng::StatManager::createStat("Force", 20, "(val - 10) / 2");
    dng::StatManager::createStat("Dexterité", 20, "(val - 10) / 2");
    dng::StatManager::createStat("Constitution", 20, "(val - 10) / 2");
    dng::StatManager::createStat("Intelligence", 20, "(val - 10) / 2");
    dng::StatManager::createStat("Sagesse", 20, "(val - 10) / 2");
    dng::StatManager::createStat("Charisme", 20, "(val - 10) / 2");
    TempPlayer player;

    int32_t difficulty = 0;
    std::println("Choose your difficulty between 0 and 20");
    std::cin >> difficulty;
    std::println("The difficulty is {} try to go above:", difficulty);

    std::string input;
    while (1) {
        std::println("Roll a stat:");
        std::cin >> input;

        auto res = player.getStat(input);
        if (res) {
            if (dng::StatManager::roll(input, *res) >= difficulty)
                std::println("You succeded !");
            else
                std::println("Failed...");
        }
        else std::println("error: {}", res.error());
        std::println();
    }
    return 0;
}
