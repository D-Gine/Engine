/*
 * Copyright 2026 <D&Gine Group>
 */

#include "ECS/DenseSA.hpp"
#include "ECS/DenseZipper.hpp"
#include "ECS/Entity.hpp"
#include "Engine/LuaTNumber.hpp"
#include "Engine/LuaTString.hpp"
#include "LuaContext.hpp"
#include <LuaCpp.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "ECS/Registry.hpp"

struct TestComponent {
    int value;
};


int main() {
    ECS::Registry registry;
    LuaCpp::LuaContext ctx;

    registry.registerComponent<TestComponent>();
    registry.addComponent(67, TestComponent{0});

    ECS::DenseSparseArray<TestComponent> &tests = registry.getComponents<TestComponent>();
    std::shared_ptr<LuaCpp::Engine::LuaTNumber> val = std::make_shared<LuaCpp::Engine::LuaTNumber>(tests[67].value);
    
    ctx.AddGlobalVariable("value", val);
    try {
        ctx.CompileFile("test", "test.lua");
    } catch (const std::exception& e) {
        std::cerr << "Error during compilation: " << e.what() << std::endl;
        return 1;
    }

    std::chrono::steady_clock::time_point lastCheck = std::chrono::steady_clock::now();


    auto lastWrite = std::filesystem::last_write_time("test.lua");

    while (1) {
        if (std::chrono::steady_clock::now() - lastCheck > std::chrono::seconds(1)) {
            lastCheck = std::chrono::steady_clock::now();
            auto currentWrite = std::filesystem::last_write_time("test.lua");
            if (currentWrite != lastWrite) {
                std::cout << "File changed, recompiling and running...\n";
                lastWrite = currentWrite;
                try {
                    ctx.CompileFile("test", "test.lua", true);
                } catch (const std::exception& e) {
                    std::cerr << "Error during compilation: " << e.what() << std::endl;
                }
            }
        }
        try {
            ctx.RunPooled("test");
            tests[67].value = val->getValue();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        for (auto &&[test] : ECS::DenseZipper(tests)) {
            std::cout << std::format("val = {}\n", test.value);
        }
    }
    return 0;
}
