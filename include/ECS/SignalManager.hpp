/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <typeindex>
    #include <utility>
    #include <unordered_map>
    #include <functional>
    #include <vector>
    #include <memory>

namespace dng {

/**
 * @brief Event-driven signal/slot system using the Observer pattern
 *
 * SignalManager provides a type-safe, decoupled communication mechanism
 * between different parts of your application without direct dependencies.
 *
 * Key Concepts:
 * - Signal: An event type (e.g., PlayerDied, ScoreChanged, CollisionDetected)
 * - Slot/Callback: A function that responds to a specific signal type
 * - Subscribe: Register a callback to listen for a signal type
 * - Emit: Trigger all callbacks registered for a signal type
 *
 * Benefits:
 * - Decoupling: Components don't need to know about each other
 * - Flexibility: Add/remove listeners at runtime
 * - Type-safety: Each signal type has its own callback signature
 *
 * Architecture Pattern: Observer Pattern
 * - Subject (SignalManager) notifies observers (callbacks) when events occur
 * - Observers can be enabled/disabled dynamically
 * - Multiple observers can listen to the same event
 *
 * @example Basic usage
 * @code
 * SignalManager manager;
 *
 * // Define event types
 * struct PlayerDied { int player_id; };
 * struct ScoreChanged { int old_score, new_score; };
 *
 * // Subscribe to events
 * auto id1 = manager.sub<PlayerDied>([](PlayerDied event) {
 *     std::println("Player {} died!", event.player_id);
 * });
 *
 * auto id2 = manager.sub<ScoreChanged>([](ScoreChanged event) {
 *     std::println("Score: {} -> {}", event.old_score, event.new_score);
 * });
 *
 * // Emit events
 * manager.emit(PlayerDied{1});           // Triggers callback
 * manager.emit(ScoreChanged{10, 15});    // Triggers callback
 *
 * // Control callbacks
 * manager.disableCallback(id1);
 * manager.emit(PlayerDied{2});           // No output (disabled)
 * manager.enableCallback(id1);
 * manager.emit(PlayerDied{3});           // Triggers callback again
 * @endcode
 *
 * @example Real-world ECS integration
 * @code
 * struct CollisionEvent {
 *     Entity entity_a;
 *     Entity entity_b;
 * };
 *
 * Registry registry;  // Registry inherits from SignalManager
 *
 * // Physics system emits collisions
 * auto physics_system = [&]() {
 *     // ... detect collision ...
 *     registry.emit(CollisionEvent{player, enemy});
 * };
 *
 * // Damage system responds to collisions
 * registry.sub<CollisionEvent>([&](CollisionEvent e) {
 *     auto health = registry.getComponents<Health>();
 *     if (health.has_value()) {
 *         health->get().getComponent(e.entity_a).hp -= 10;
 *     }
 * });
 *
 * // Sound system also responds to collisions
 * registry.sub<CollisionEvent>([&](CollisionEvent e) {
 *     playSound("collision.wav");
 * });
 * @endcode
 */
class SignalManager {
 public:
    /**
     * @brief Unique identifier for registered callbacks
     *
     * Use this ID to enable/disable specific callbacks at runtime.
     * IDs are globally unique and monotonically increasing.
     */
    using CallbackId = std::size_t;

 public:
    /**
     * @brief Subscribe a callback to a specific event type
     *
     * This function registers a callback that will be invoked whenever
     * an event of type `Type` is emitted. The callback receives the
     * event as a parameter.
     *
     * Features:
     * - Type-safe: Callback signature must match event type
     * - Multiple callbacks: You can register multiple callbacks for the same event
     * - Lazy initialization: Signal storage is created on first subscription
     * - Perfect forwarding: Efficiently accepts any callable (lambda, function pointer, functor)
     *
     * Thread-safety: Not thread-safe. Don't call from multiple threads.
     *
     * @tparam Type The event type to subscribe to (e.g., PlayerDied, ScoreChanged)
     * @tparam Func The callable type (deduced automatically)
     * @param func Callback function with signature: void(Type)
     * @return CallbackId Unique ID for this callback (use to enable/disable later)
     *
     * @example Different callback types
     * @code
     * SignalManager mgr;
     *
     * struct MyEvent { int value; };
     *
     * // Lambda
     * auto id1 = mgr.sub<MyEvent>([](MyEvent e) {
     *     std::println("Lambda: {}", e.value);
     * });
     *
     * // Function pointer
     * void myHandler(MyEvent e) { std::println("Function: {}", e.value); }
     * auto id2 = mgr.sub<MyEvent>(myHandler);
     *
     * // Capturing lambda
     * int counter = 0;
     * auto id3 = mgr.sub<MyEvent>([&counter](MyEvent e) {
     *     counter++;
     *     std::println("Count: {}", counter);
     * });
     *
     * // Functor/callable object
     * struct Handler {
     *     void operator()(MyEvent e) { std::println("Functor: {}", e.value); }
     * };
     * auto id4 = mgr.sub<MyEvent>(Handler{});
     * @endcode
     *
     * @example Chaining events (cascading)
     * @code
     * SignalManager mgr;
     *
     * struct EventA { int data; };
     * struct EventB { int data; };
     *
     * // EventA triggers EventB
     * mgr.sub<EventA>([&mgr](EventA e) {
     *     std::println("A: {}", e.data);
     *     mgr.emit(EventB{e.data * 2});  // Cascade to EventB
     * });
     *
     * mgr.sub<EventB>([](EventB e) {
     *     std::println("B: {}", e.data);
     * });
     *
     * mgr.emit(EventA{5});  // Prints "A: 5" then "B: 10"
     * @endcode
     */
    template<typename Type, typename Func>
    CallbackId sub(Func&& func) {
        if (_subs.find(typeid(Type)) == _subs.end()) {
            _subs.emplace(typeid(Type), std::make_unique<SignalList<Type>>());
        }

        auto* list = static_cast<SignalList<Type>*>(
            _subs.at(typeid(Type)).get());

        CallbackId id = _next_callback_id++;
        list->callbacks.push_back({
            id,
            std::function<void(Type)>(std::forward<Func>(func)),
            true  // Enabled by default
        });
        return id;
    }

    /**
     * @brief Enable a previously disabled callback
     *
     * Re-activates a callback that was disabled with disableCallback().
     * If the callback is already enabled, this is a no-op.
     * If the ID doesn't exist, this is a no-op (no error).
     *
     * Use case: Temporarily disable callbacks during loading, initialization,
     * or specific game states, then re-enable them later.
     *
     * Performance: O(n) where n is total number of callbacks across all types.
     *
     * @param id The callback ID returned from sub()
     *
     * @example Pause/resume system
     * @code
     * SignalManager mgr;
     *
     * struct GameEvent { std::string message; };
     *
     * auto ui_callback = mgr.sub<GameEvent>([](GameEvent e) {
     *     std::println("UI: {}", e.message);
     * });
     *
     * // During game pause
     * mgr.disableCallback(ui_callback);
     * mgr.emit(GameEvent{"something happened"});  // No output
     *
     * // Resume game
     * mgr.enableCallback(ui_callback);
     * mgr.emit(GameEvent{"game resumed"});  // Output: "UI: game resumed"
     * @endcode
     */
    void enableCallback(CallbackId id) {
        for (auto& [name, signal_list] : _subs) {
            signal_list->enableCallback(id);
        }
    }

    /**
     * @brief Disable a callback without removing it
     *
     * Temporarily deactivates a callback. The callback remains registered
     * but won't be invoked when events are emitted.
     *
     * This is more efficient than unsubscribing and re-subscribing if you
     * need to toggle callbacks frequently.
     *
     * Performance: O(n) where n is total number of callbacks across all types.
     *
     * @param id The callback ID returned from sub()
     *
     * @example Conditional event handling
     * @code
     * SignalManager mgr;
     *
     * struct DebugEvent { std::string message; };
     *
     * auto debug_callback = mgr.sub<DebugEvent>([](DebugEvent e) {
     *     std::println("[DEBUG] {}", e.message);
     * });
     *
     * bool debug_mode = false;
     *
     * void toggleDebugMode() {
     *     debug_mode = !debug_mode;
     *     if (debug_mode) {
     *         mgr.enableCallback(debug_callback);
     *     } else {
     *         mgr.disableCallback(debug_callback);
     *     }
     * }
     * @endcode
     */
    void disableCallback(CallbackId id) {
        for (auto& [name, signal_list] : _subs) {
            signal_list->disableCallback(id);
        }
    }

    /**
     * @brief Emit an event, triggering all enabled callbacks
     *
     * Invokes all enabled callbacks registered for this event type.
     * The event is passed by value to each callback.
     *
     * Execution order:
     * - Callbacks are invoked in registration order
     * - All enabled callbacks complete before emit() returns
     * - Synchronous execution (no threading/async behavior)
     *
     * If no callbacks are registered for this type, this is a no-op.
     *
     * Performance: O(n) where n is number of callbacks for this event type.
     *
     * @tparam Type The event type (deduced from parameter)
     * @param event The event instance to emit
     *
     * @example Event broadcasting
     * @code
     * SignalManager mgr;
     *
     * struct PlayerScored {
     *     int player_id;
     *     int points;
     * };
     *
     * // Multiple systems listen to the same event
     * mgr.sub<PlayerScored>([](PlayerScored e) {
     *     // UI system updates scoreboard
     *     updateScoreboard(e.player_id, e.points);
     * });
     *
     * mgr.sub<PlayerScored>([](PlayerScored e) {
     *     // Achievement system checks for milestones
     *     checkAchievements(e.player_id, e.points);
     * });
     *
     * mgr.sub<PlayerScored>([](PlayerScored e) {
     *     // Sound system plays audio
     *     if (e.points >= 100) {
     *         playSound("big_score.wav");
     *     }
     * });
     *
     * // One emit triggers all three callbacks
     * mgr.emit(PlayerScored{1, 150});
     * @endcode
     *
     * @example Event with no listeners (safe)
     * @code
     * struct UnusedEvent { int data; };
     * mgr.emit(UnusedEvent{42});  // No error, just no-op
     * @endcode
     */
    template<typename Type>
    void emit(Type event) {
        // No callbacks registered for this type? No-op
        if (_subs.find(typeid(Type)) == _subs.end()) {
            return;
        }

        // Get the concrete signal list for this type
        auto* list = static_cast<SignalList<Type>*>(
            _subs.at(typeid(Type)).get());

        // Invoke all enabled callbacks
        for (auto& callback : list->callbacks) {
            if (callback.enabled) {
                callback.func(event);
            }
        }
    }

 private:
    /**
     * @brief Type-erased base interface for signal lists
     *
     * Design Pattern: Type Erasure
     * - Problem: We need to store different SignalList<T> types in one container
     * - Solution: Abstract base class with virtual functions
     * - Benefits: Allows enable/disable operations without knowing concrete type
     *
     * This interface provides operations that don't need type information
     * (enable/disable by ID works for any event type).
     */
    struct ISignalList {
        virtual ~ISignalList() = default;

        /// Enable a callback by ID (type-agnostic operation)
        virtual void enableCallback(CallbackId id) = 0;

        /// Disable a callback by ID (type-agnostic operation)
        virtual void disableCallback(CallbackId id) = 0;
    };

    /**
     * @brief Type-specific storage for callbacks of a particular event type
     *
     * Each event type (e.g., PlayerDied, ScoreChanged) gets its own
     * SignalList<T> instance storing callbacks for that specific type.
     *
     * Template allows type-safe callback storage and invocation.
     *
     * @tparam Args Event type parameter pack (typically single type)
     */
    template<typename... Args>
    struct SignalList : ISignalList {
        /**
         * @brief Container for a single callback with metadata
         *
         * Stores:
         * - id: Unique identifier for enable/disable operations
         * - func: The actual callback function
         * - enabled: Runtime flag to control invocation
         */
        struct Callback {
            CallbackId id;                      ///< Unique callback identifier
            std::function<void(Args...)> func;  ///< Type-safe callback function
            bool enabled;                       ///< Is this callback active?
        };
        std::vector<Callback> callbacks;

        /**
         * @brief Enable a callback by ID
         *
         * Searches for matching ID and sets enabled flag to true.
         * Stops after first match (IDs are unique).
         */
        void enableCallback(CallbackId id) override {
            for (auto& cb : callbacks) {
                if (cb.id == id) {
                    cb.enabled = true;
                    break;
                }
            }
        }

        /**
         * @brief Disable a callback by ID
         *
         * Searches for matching ID and sets enabled flag to false.
         * Stops after first match (IDs are unique).
         */
        void disableCallback(CallbackId id) override {
            for (auto& cb : callbacks) {
                if (cb.id == id) {
                    cb.enabled = false;
                    break;
                }
            }
        }
    };

    /**
     * @brief Storage for all signal lists, keyed by event type
     *
     * Design Choice: std::type_index as key
     * - Allows runtime type identification and lookup
     * - Each unique event type gets its own SignalList<T>
     * - type_index is hashable, making unordered_map efficient
     *
     * Design Choice: std::unique_ptr<ISignalList>
     * - Polymorphic storage (base pointer, derived types)
     * - Unique ownership semantics (SignalManager owns the lists)
     * - Enables type erasure pattern
     *
     * Example structure:
     * {
     *   typeid(PlayerDied) -> SignalList<PlayerDied>,
     *   typeid(ScoreChanged) -> SignalList<ScoreChanged>,
     *   typeid(Collision) -> SignalList<Collision>
     * }
     */
    std::unordered_map<std::type_index, std::unique_ptr<ISignalList>> _subs;

    /**
     * @brief Global counter for generating unique callback IDs
     *
     * Incremented each time a callback is registered.
     * Guarantees uniqueness across all event types.
     */
    CallbackId _next_callback_id = 0;
};

}  // namespace dng
