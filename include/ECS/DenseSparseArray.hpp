/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <utility>
    #include <iostream>
    #include <vector>

    #include "ECS/Entity.hpp"
    #include "ECS/SparseArray.hpp"

namespace dng {

#define MAX_PAGE_SIZE 1000
#define PAGE(e) e / MAX_PAGE_SIZE
#define PAGE_INDEX(e) e % MAX_PAGE_SIZE

template <typename Component>
class DenseSparseArray {
 public:
    using value_type = Component;

    DenseSparseArray() = default;

    template <typename ...Args>
    void createComponent(Entity e, Args&&... args) {
        if (PAGE(e) >= _spar.size()) {
            _spar.resize(PAGE(e) + 1);
        }
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.emplace_back(std::forward<Args>(args)...);
        _dense_to_entity.push_back(e);
    }

    void addComponent(Entity e, Component&& cmpt) {
        if (PAGE(e) >= _spar.size()) {
            _spar.resize(PAGE(e) + 1);
        }
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.push_back(std::forward<Component>(cmpt));
        _dense_to_entity.push_back(e);
    }

    void addComponent(Entity e, const Component& cmpt) {
        if (PAGE(e) >= _spar.size()) {
            _spar.resize(PAGE(e) + 1);
        }
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.push_back(cmpt);
        _dense_to_entity.push_back(e);
    }

    bool hasComponent(Entity e) {
        return (PAGE(e) < _spar.size() &&
            PAGE_INDEX(e) <_spar[PAGE(e)].size() &&
            _spar[PAGE(e)][PAGE_INDEX(e)].has_value());
    }

    void removeComponent(Entity e) {
        if (!hasComponent(e))
            return;
        std::size_t del_index = _spar[PAGE(e)][PAGE_INDEX(e)].value();
        Entity back_e = _dense_to_entity.back();
        if (del_index != _dense.size() - 1) {
            std::swap(_dense[del_index], _dense.back());
            std::swap(_dense_to_entity[del_index], _dense_to_entity.back());
            _spar[PAGE(back_e)][PAGE_INDEX(back_e)] = del_index;
        }
        _spar[PAGE(e)][PAGE_INDEX(e)] = std::nullopt;
        _dense.pop_back();
        _dense_to_entity.pop_back();
    }

    std::vector<SparseArray<std::size_t>>& getSpar() {
        return _spar;
    }

    const std::vector<SparseArray<std::size_t>>& getSpar() const {
        return _spar;
    }

    Component& getComponentDense(std::size_t idx) {
        return _dense[idx];
    }

    const Component& getComponentDense(std::size_t idx) const {
        return _dense[idx];
    }

    Component& getComponent(Entity e) {
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

    const Component& getComponent(Entity e) const {
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

 private:
    std::vector<Component> _dense;
    std::vector<Entity> _dense_to_entity;
    std::vector<SparseArray<std::size_t>> _spar;
};

}  // namespace dng
