/*
 * Copyright 2026 <D&Gine Group>
 */

#include <gtest/gtest.h>
#include <functional>

#include "ECS/Registry.hpp"
#include "ECS/DenseSparseArray.hpp"

struct FakeComponent {
    short fake;
};

TEST(registry, register_component) {
    dng::Registry reg = {};
    auto result = reg.registerComponent<int>();

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(typeid(std::expected<std::reference_wrapper<
        dng::DenseSparseArray<int>>, dng::TypeNotRegistred>), typeid(result));
}

TEST(registry, create_component) {
    dng::Registry reg = {};
    auto integers = reg.registerComponent<int>();
    int thousen = 1000;

    EXPECT_TRUE(integers.has_value());

    reg.createComponent<int>(0, 1);
    EXPECT_TRUE(integers.value().get().getSpar()[0][0].has_value());
    EXPECT_EQ(integers.value().get().getSpar()[0][0].value(), 0);
    EXPECT_EQ(integers.value().get().getComponent(0), 1);

    reg.addComponent<int>(1, 2);
    EXPECT_TRUE(integers.value().get().getSpar()[0][1].has_value());
    EXPECT_EQ(integers.value().get().getSpar()[0][1].value(), 1);
    EXPECT_EQ(integers.value().get().getComponent(1), 2);

    reg.addComponent<int>(1000, thousen);
    EXPECT_TRUE(integers.value().get().getSpar()[1][0].has_value());
    EXPECT_EQ(integers.value().get().getSpar()[1][0].value(), 2);
    EXPECT_EQ(integers.value().get().getComponent(2), 1000);
}

TEST(registry, bad_create_component) {
    dng::Registry reg = {};
    int third = 3;

    reg.createComponent<int>(0, 1);
    reg.addComponent<int>(1, 2);
    reg.addComponent<int>(2, third);
}

TEST(registry, access_component) {
    // ECS::Registry reg = {};
    // auto& integers = reg.registerComponent<int>();

    // reg.addComponent<int>(0, 1);
    // EXPECT_TRUE(integers.getSpar()[0][0].has_value());
    // EXPECT_EQ(integers.getSpar()[0][0].value(), 0);
    // EXPECT_EQ(integers.getComponent(0), 1);
    // reg.createComponent<int>(2, 3);
    // EXPECT_TRUE(integers.getSpar()[0][2].has_value());
    // EXPECT_EQ(integers.getSpar()[0][2].value(), 1);
    // EXPECT_EQ(integers.getComponent(1), 3);
}

TEST(registry, bad_access_component) {
    // ECS::Registry reg = {};

    // EXPECT_THROW(reg.getComponents<int>(), std::out_of_range);
}

TEST(registry, remove_entity_components) {
    // ECS::Registry reg = {};
    // auto& integers = reg.registerComponent<int>();

    // reg.addComponent<int>(0, 1);
    // reg.addComponent<int>(1, 2);
    // reg.addComponent<int>(2, 3);
    // reg.killEntity(0);
    // reg.removeComponent<int>(2);
    // EXPECT_FALSE(integers.getSpar()[0][0].has_value());
    // EXPECT_FALSE(integers.getSpar()[0][2].has_value());
    // EXPECT_EQ(integers.getComponent(integers.getSpar()[0][1].value()), 2);
}

TEST(registry, systems) {
    // ECS::Registry reg = {};
    // auto& integers = reg.registerComponent<int>();

    // reg.addComponent<int>(0, 1);
    // reg.addComponent<int>(1000, 1000);
    // reg.addSystem(test_system_add_one);
    // reg.runSystems();
    // EXPECT_TRUE(integers.getSpar()[0][0].has_value());
    // EXPECT_EQ(integers.getSpar()[0][0].value(), 0);
    // EXPECT_EQ(integers.getComponent(0), 2);
    // EXPECT_TRUE(integers.getSpar()[1][0].has_value());
    // EXPECT_EQ(integers.getSpar()[1][0].value(), 1);
    // EXPECT_EQ(integers.getComponent(1), 1001);
}
