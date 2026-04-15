/*
 * Copyright 2026 <D&Gine Group>
 */

#include <string>
#include "ECS/Registry.hpp"

namespace dng {

void Registry::killEntity(Entity e) {
    for (auto& rm : _remover)
        rm(*this, e);
}

void Registry::runSystems(void) {
    for (const auto& [name, sys] : _systems)
        sys(*this);
}

void Registry::addSystem(const std::string& name,
    const std::function<void(Registry&)>& f) {
    _systems.emplace_back(name, f);
}

int Registry::getSystem(const std::string& name) {
    for (int i = 0; i < _systems.size(); ++i)
        if (!name.compare(_systems[i].first))
            return i;
    return -1;
}

void Registry::clearSystems(void) {
    _systems.clear();
}

void Registry::removeSystem(const std::string& name) {
    auto idx = getSystem(name);
    if (idx < 0)
        return;
    _systems.erase(_systems.begin() + idx);
}

}  // namespace dng
