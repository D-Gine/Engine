/*
 * Copyright 2026 <D&Gine Group>
 */

#include <print>

// #include "Server.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Zipper.hpp"

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

    auto positions = reg.getComponents<Position>();
    if (!positions.has_value())
        std::println("{}", positions.error().what());
    auto velocities = reg.getComponents<Velocity>();
    if (!velocities.has_value())
        std::println("{}", velocities.error().what());
    auto fake = reg.getComponents<FakeComponent>();
    if (!fake.has_value())
        std::println("{}", fake.error().what());

    reg.createComponent<Position>(10, 1.f, 1.f);
    reg.createComponent<Velocity>(10, 1.f, 1.f);

    FakeComponent fake_cmpt;
    reg.createComponent<FakeComponent>(11, 1);
    reg.addComponent<FakeComponent>(11, FakeComponent());
    reg.addComponent<FakeComponent>(11, fake_cmpt);

    auto id_int = reg.sub<int>([&reg](int value) {
        std::println("int: {}", value);
        reg.emit<float>(value);
    });
    auto id_float = reg.sub<float>([&reg](float value) {
        std::println("float: {}", value);
    });
    reg.emit<int>(1);
    reg.emit<FakeComponent>(fake_cmpt);
    reg.disableCallback(id_float);
    reg.emit<int>(2);
    reg.enableCallback(id_float);
    reg.emit<int>(3);
    reg.disableCallback(400);
    for (auto&& [e, pos, vel] : dng::IndexedZipper(*positions, *velocities)) {
        std::println("found [{}] {}-{} and {}-{}",
            e, pos.x, pos.y, vel.x, vel.y);
    }
    return 0;
}
