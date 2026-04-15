/*
 * Copyright 2026 <D&Gine Group>
 */

#include <print>

// #include "Server.hpp"
#include "ECS/Registry.hpp"

struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct FakeComponent {
    float x;
    float y;
};

int main() {
    // dng::Server server;
    // server.start();
    dng::Registry reg;

    reg.registerComponent<Position>();
    reg.registerComponent<Velocity>();

    auto pos = reg.getComponents<Position>();
    if (!pos.has_value())
        std::println("{}", pos.error().what());
    auto vel = reg.getComponents<Velocity>();
    if (!vel.has_value())
        std::println("{}", vel.error().what());
    auto fake = reg.getComponents<FakeComponent>();
    if (!fake.has_value())
        std::println("{}", fake.error().what());

    reg.createComponent<Position>(10, 1.f, 1.f);
    reg.createComponent<Velocity>(10, 1.f, 1.f);

    reg.createComponent<FakeComponent>(11, 1);
    return 0;
}
