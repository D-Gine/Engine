/*
 * Copyright 2026 <D&Gine Group>
 */

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"

namespace dng {

void Registry::killEntity(Entity e) {
    for (auto& rm : _remover)
        rm(*this, e);
}

}  // namespace dng
