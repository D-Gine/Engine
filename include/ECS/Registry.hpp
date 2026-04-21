/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <unordered_map>
    #include <typeindex>
    #include <functional>
    #include <any>
    #include <vector>
    #include <utility>
    #include <string>
    #include <expected>
    #include <exception>
    #include <print>

    #include "ECS/DenseSparseArray.hpp"
    #include "ECS/Entity.hpp"
    #include "ECS/SignalManager.hpp"

namespace dng {

class TypeNotRegistred : public std::exception {
 public:
    explicit TypeNotRegistred(const std::string& type) {
        _message = std::format("'{}' not registred", type);
    }
    const char *what() const noexcept { return _message.c_str(); }
 private:
    std::string _message;
};

struct ComponentListStorer :
    public std::unordered_map<std::type_index, std::any> {
    template<typename Component>
    std::expected<std::reference_wrapper<DenseSparseArray<Component>>,
        TypeNotRegistred> getSparseArray() {
        if (find(typeid(Component)) == end())
            return std::unexpected(TypeNotRegistred(typeid(Component).name()));
        return std::any_cast<DenseSparseArray<Component>&>(
            at(typeid(Component)));
    }

    template<typename Component>
    std::expected<std::reference_wrapper<DenseSparseArray<Component>>,
        TypeNotRegistred> getSparseArray() const {
        if (find(typeid(Component)) == end())
            return std::unexpected(TypeNotRegistred(typeid(Component).name()));
        return std::any_cast<DenseSparseArray<Component>&>(
            at(typeid(Component)));
    }
};

class Registry : public SignalManager{
 public:
    using ComponentRemover =
        std::vector<std::function<void(Registry&, const Entity&)>>;
    using SystemList =
        std::vector<std::pair<std::string, std::function<void(Registry&)>>>;

 public:
    template <typename Component>
    std::expected<std::reference_wrapper<DenseSparseArray<Component>>,
        TypeNotRegistred> registerComponent() {
        _components.insert_or_assign(
            std::type_index(typeid(Component)), DenseSparseArray<Component>());
        _remover.push_back([](Registry& reg, Entity e) {
            auto components = reg.getComponents<Component>();
            if (!components.has_value()) {
                std::println("Error: could not remove component: {}",
                    components.error().what());
                return;
            }
            components.value().get().removeComponent(e);
        });
        return _components.getSparseArray<Component>();
    }

    template <typename Component>
    std::expected<std::reference_wrapper<DenseSparseArray<Component>>,
        TypeNotRegistred> getComponents() {
        return _components.getSparseArray<Component>();
    }

    template <typename Component>
    std::expected<std::reference_wrapper<DenseSparseArray<Component>>,
        TypeNotRegistred> const & getComponents() const {
        return _components.getSparseArray<Component>();
    }

    void killEntity(Entity e);

    template <typename Component, typename... Args>
    void createComponent(Entity e, Args&&... args) {
        auto list = _components.getSparseArray<Component>();
        if (!list.has_value()) {
            std::println("Error: could not create component: {}",
                list.error().what());
            return;
        }
        list.value().get().createComponent(e, std::forward<Args>(args)...);
    }

    template <typename Component>
    void addComponent(Entity e, const Component& c) {
        auto list = _components.getSparseArray<Component>();
        if (!list.has_value()) {
            std::println("Error: could not add(&) component: {}",
                list.error().what());
            return;
        }
        list.value().get().addComponent(e, c);
}

    template <typename Component>
    void addComponent(Entity e, Component&& c) {
        auto list = _components.getSparseArray<Component>();
        if (!list.has_value()) {
            std::println("Error: could not add(&&) component: {}",
                list.error().what());
            return;
        }
        list.value().get().addComponent(e, std::forward<Component>(c));
    }

    template <typename Component>
    void removeComponent(Entity e) {
        auto list = _components.getSparseArray<Component>();
        if (!list.has_value()) {
            std::println("Error: could not remove component: {}",
                list.error().what());
            return;
        }
        list.value().get().removeComponent(e);
    }

 private:
    ComponentListStorer _components;
    ComponentRemover _remover;
};

}  // namespace dng
