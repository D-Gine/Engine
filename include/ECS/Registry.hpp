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

/**
 * @brief Exception thrown when attempting to access an unregistered component type
 *
 * This error occurs when you try to get, add, or remove a component type
 * that hasn't been registered with Registry::registerComponent<T>()
 *
 * @example
 * @code
 * Registry reg;
 * // Forgot to call reg.registerComponent<Position>()
 * auto pos = reg.getComponents<Position>();  // Throws TypeNotRegistred
 * @endcode
 */
class TypeNotRegistred : public std::exception {
 public:
    explicit TypeNotRegistred(const std::string& type) {
        _message = std::format("'{}' not registred", type);
    }
    const char *what() const noexcept { return _message.c_str(); }
 private:
    std::string _message;
};

/**
 * @brief Type alias for safe component array access
 *
 * Returns either:
 * - A reference_wrapper to the component array (success case)
 * - A TypeNotRegistred error (failure case)
 *
 * Why std::reference_wrapper?
 * - std::expected cannot hold reference types directly (std::expected<T&, E> is illegal)
 * - reference_wrapper allows us to safely return references in value semantics
 * - Call .get() on the wrapper to access the underlying array
 *
 * @tparam Component The component type (e.g., Position, Velocity)
 */
template <typename Component>
using SafeArray = std::expected<
    std::reference_wrapper<DenseSparseArray<Component>>, TypeNotRegistred>;

/**
 * @brief Type-erased storage for all component arrays
 *
 * This class stores different component types in a single container using:
 * - std::type_index as the key (allows runtime type identification)
 * - std::any as the value (can hold any type, but requires casting)
 *
 * Design choice: Why std::any?
 * - We need to store different types (DenseSparseArray<Position>,
 *   DenseSparseArray<Velocity>, etc.) in the same map
 * - std::any provides type-safe storage with runtime type checking
 * - std::any_cast ensures we retrieve the correct type
 */
struct ComponentListStorer :
    public std::unordered_map<std::type_index, std::any> {

    /**
     * @brief Safely retrieve a component array by type
     *
     * @tparam Component The component type to retrieve
     * @return SafeArray<Component> containing either:
     *         - reference_wrapper to the array (use .value().get())
     *         - TypeNotRegistred error (check with .has_value())
     *
     * @example
     * @code
     * auto result = storer.getSparseArray<Position>();
     * if (result.has_value()) {
     *     auto& array = result.value().get();  // Get actual array
     *     // Use array...
     * } else {
     *     std::println("Error: {}", result.error().what());
     * }
     * @endcode
     */
    template<typename Component>
    SafeArray<Component> getSparseArray() {
        if (find(typeid(Component)) == end())
            return std::unexpected(TypeNotRegistred(typeid(Component).name()));
        return std::any_cast<DenseSparseArray<Component>&>(
            at(typeid(Component)));
    }

    /// @brief Const overload of getSparseArray
    template<typename Component>
    SafeArray<Component> getSparseArray() const {
        if (find(typeid(Component)) == end())
            return std::unexpected(TypeNotRegistred(typeid(Component).name()));
        return std::any_cast<DenseSparseArray<Component>&>(
            at(typeid(Component)));
    }
};

/**
 * @brief Central ECS (Entity Component System) Registry
 *
 * The Registry is the heart of the ECS architecture. It:
 * - Manages all component types and their storage
 * - Creates and destroys entities
 * - Provides component access for systems
 * - Handles signals/events (via SignalManager base class)
 *
 * ECS Architecture Overview:
 * - Entity: Just an ID (unsigned int), has no data or behavior
 * - Component: Pure data (e.g., Position, Velocity, Health)
 * - System: Logic that operates on entities with specific components
 * - Registry: Connects everything together
 *
 * Typical Workflow:
 * 1. Register component types: reg.registerComponent<Position>()
 * 2. Create entities with components: reg.createComponent<Position>(entity, x, y)
 * 3. Query components: reg.getComponents<Position>()
 * 4. Iterate with systems: for (auto&& [pos, vel] : Zipper(positions, velocities))
 *
 * @example Basic usage
 * @code
 * Registry reg;
 *
 * // 1. Register component types
 * reg.registerComponent<Position>();
 * reg.registerComponent<Velocity>();
 *
 * // 2. Create entity with components
 * Entity player = 0;
 * reg.createComponent<Position>(player, 0.0f, 0.0f);
 * reg.createComponent<Velocity>(player, 1.0f, 0.5f);
 *
 * // 3. Access components in a system
 * auto positions = reg.getComponents<Position>();
 * auto velocities = reg.getComponents<Velocity>();
 * if (positions && velocities) {
 *     for (auto&& [pos, vel] : Zipper(*positions, *velocities)) {
 *         pos.x += vel.x;  // Move entities
 *         pos.y += vel.y;
 *     }
 * }
 * @endcode
 */
class Registry : public SignalManager {
 public:
    /**
     * @brief Type for storing component removal callbacks
     *
     * When an entity is killed, all its components must be removed.
     * This vector stores lambda functions that know how to remove
     * each registered component type.
     */
    using ComponentRemover =
        std::vector<std::function<void(Registry&, const Entity&)>>;

    /**
     * @brief Type for storing game systems
     *
     * Systems are functions that operate on entities with specific components.
     * Each system has a name (for debugging) and a function pointer.
     */
    using SystemList =
        std::vector<std::pair<std::string, std::function<void(Registry&)>>>;

 public:
    /**
     * @brief Register a new component type with the registry
     *
     * This MUST be called before using any component type. It:
     * 1. Creates storage (DenseSparseArray) for this component type
     * 2. Registers a removal callback for entity cleanup
     * 3. Returns a reference to the created storage
     *
     * Thread-safety: Not thread-safe. Call during initialization only.
     *
     * @tparam Component The component type to register (e.g., Position)
     * @return SafeArray<Component> reference to the component storage
     *
     * @example
     * @code
     * struct Position { float x, y; };
     * struct Velocity { float dx, dy; };
     *
     * Registry reg;
     * reg.registerComponent<Position>();  // Must call before using Position
     * reg.registerComponent<Velocity>();  // Must call before using Velocity
     * @endcode
     */
    template <typename Component>
    SafeArray<Component> registerComponent() {
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

    /**
     * @brief Get access to all components of a specific type
     *
     * Use this to retrieve the component array for iteration or queries.
     * Typically used with Zipper to iterate over multiple component types.
     *
     * @tparam Component The component type to retrieve
     * @return SafeArray<Component> Either the array or an error
     *
     * @example
     * @code
     * auto positions = reg.getComponents<Position>();
     * if (!positions.has_value()) {
     *     std::println("Error: {}", positions.error().what());
     *     return;
     * }
     *
     * // Use with Zipper for multi-component iteration
     * auto velocities = reg.getComponents<Velocity>();
     * for (auto&& [pos, vel] : Zipper(*positions, *velocities)) {
     *     pos.x += vel.x;
     * }
     * @endcode
     */
    template <typename Component>
    SafeArray<Component> getComponents() {
        return _components.getSparseArray<Component>();
    }

    /// @brief Const overload of getComponents
    template <typename Component>
    SafeArray<Component> const & getComponents() const {
        return _components.getSparseArray<Component>();
    }

    /**
     * @brief Destroy an entity and remove all its components
     *
     * This function:
     * 1. Iterates through all registered component types
     * 2. Removes the component from each array if present
     * 3. Frees the entity ID for reuse
     *
     * @param e The entity to destroy
     *
     * @example
     * @code
     * Entity enemy = 42;
     * reg.createComponent<Position>(enemy, 10.0f, 5.0f);
     * reg.createComponent<Health>(enemy, 100);
     *
     * // Later, when enemy dies:
     * reg.killEntity(enemy);  // Removes Position and Health automatically
     * @endcode
     */
    void killEntity(Entity e);

    /**
     * @brief Create a component in-place using constructor arguments
     *
     * This is the most efficient way to add components. It constructs
     * the component directly in the array without copying or moving.
     *
     * Perfect forwarding ensures constructor arguments are passed efficiently.
     *
     * @tparam Component The component type to create
     * @tparam Args Variadic template for constructor arguments
     * @param e The entity to attach the component to
     * @param args Constructor arguments forwarded to Component's constructor
     *
     * @example
     * @code
     * struct Position {
     *     Position(float x, float y) : x(x), y(y) {}
     *     float x, y;
     * };
     *
     * Entity player = 0;
     * // Calls Position(10.0f, 20.0f) constructor directly
     * reg.createComponent<Position>(player, 10.0f, 20.0f);
     * @endcode
     */
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

    /**
     * @brief Add a component by copying an existing instance
     *
     * Use when you have a component instance you want to copy.
     * The component is copied into the array.
     *
     * @tparam Component The component type
     * @param e The entity to attach the component to
     * @param c The component instance to copy
     *
     * @example
     * @code
     * Position template_pos{100.0f, 200.0f};
     *
     * // Copy template_pos to multiple entities
     * reg.addComponent<Position>(entity1, template_pos);
     * reg.addComponent<Position>(entity2, template_pos);
     * @endcode
     */
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

    /**
     * @brief Add a component by moving an existing instance
     *
     * Use when you have a temporary or movable component instance.
     * More efficient than copying for large components.
     * The component is moved into the array (source is invalidated).
     *
     * @tparam Component The component type
     * @param e The entity to attach the component to
     * @param c The component instance to move (rvalue reference)
     *
     * @example
     * @code
     * // Move a temporary directly
     * reg.addComponent<Position>(entity, Position{50.0f, 75.0f});
     *
     * // Or explicitly move
     * Position pos{10.0f, 20.0f};
     * reg.addComponent<Position>(entity, std::move(pos));
     * // pos is now in undefined state, don't use it!
     * @endcode
     */
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

    /**
     * @brief Remove a component from an entity
     *
     * If the entity doesn't have this component, the operation is a no-op.
     * The component's destructor is called and its storage is freed.
     *
     * @tparam Component The component type to remove
     * @param e The entity to remove the component from
     *
     * @example
     * @code
     * // Remove gravity from a flying enemy
     * reg.removeComponent<Gravity>(flying_enemy);
     *
     * // Remove health when entity becomes invulnerable
     * reg.removeComponent<Health>(god_mode_entity);
     * @endcode
     */
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
