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

class SignalManager {
 public:
    using CallbackId = std::size_t;

 public:
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
            true
        });
        return id;
    }

    void enableCallback(CallbackId id) {
        for (auto& [name, signal_list] : _subs) {
            signal_list->enableCallback(id);
        }
    }

    void disableCallback(CallbackId id) {
        for (auto& [name, signal_list] : _subs) {
            signal_list->disableCallback(id);
        }
    }

    template<typename Type>
    void emit(Type event) {
        if (_subs.find(typeid(Type)) == _subs.end()) {
            return;
        }
        auto* list = static_cast<SignalList<Type>*>(
            _subs.at(typeid(Type)).get());
        for (auto& callback : list->callbacks) {
            if (callback.enabled) {
                callback.func(event);
            }
        }
    }

 private:
    struct ISignalList {
        virtual ~ISignalList() = default;
        virtual void enableCallback(CallbackId id) = 0;
        virtual void disableCallback(CallbackId id) = 0;
    };

    template<typename... Args>
    struct SignalList : ISignalList {
        struct Callback {
            CallbackId id;
            std::function<void(Args...)> func;
            bool enabled;
        };
        std::vector<Callback> callbacks;


        void enableCallback(CallbackId id) override {
            for (auto& cb : callbacks) {
                if (cb.id == id) {
                    cb.enabled = true;
                    break;
                }
            }
        }

        void disableCallback(CallbackId id) override {
            for (auto& cb : callbacks) {
                if (cb.id == id) {
                    cb.enabled = false;
                    break;
                }
            }
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<ISignalList>> _subs;
    CallbackId _next_callback_id = 0;
};

}  // namespace dng
