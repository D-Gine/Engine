/*
 * Copyright 2026 <D&Gine Group>
 */

#include <print>
#include <iostream>

#include <Server.hpp>
#include <RuleSet.hpp>
#include <Entity.hpp>

int main() {
    srand(time(0));
    dng::RuleSet::init("tests/ruleset.json");

    dng::Server server;
    server.add_handler("/hi", [](const httplib::Request&, httplib::Response& res){
        res.set_content("Hello World!", "text/plain");
        res.status = 200;
    });
    server.add_handler("/health", [](const httplib::Request&, httplib::Response& res){
        res.set_content("OK", "text/plain");
        res.status = 200;
    });

    try {
        server.start_async();
    } catch (const std::exception &e) {
        std::println(stderr, "Failed to start server: {}",  e.what());
    }

    std::string input;
    while (std::cin >> input) {
        if (input == "exit" || input == "quit") {
            break;
        }

        auto res = dng::RuleSet::entities[0].roll(input);
        if (res)
            std::println("You have rolled {} + {} ({}) in {}.",
                (*res).first, (*res).second, (*res).first + (*res).second, input);
        else
            std::println("Error: {}", res.error());
    }
    server.stop();
    server.join();
    return 0;
}
