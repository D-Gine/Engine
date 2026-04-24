/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <vector>
    #include <print>

    #include "ECS/Entity.hpp"
    #include "ECS/SparseArray.hpp"
    #include "SafeReference.hpp"

namespace dng {

#define MAX_PAGE_SIZE 1000
#define PAGE(e) e / MAX_PAGE_SIZE
#define PAGE_INDEX(e) e % MAX_PAGE_SIZE

template <typename Component>
class DenseSparseArray {
 public:
    using value_type = std::optional<Component>;
    using reference_type = OptionalRef<Component>;
    using const_reference_type = value_type const;
    using size_type = typename dng::Entity;

    using iterator = typename std::vector<size_type>::iterator;
    using const_iterator = typename std::vector<size_type>::const_iterator;

    DenseSparseArray() = default;
    DenseSparseArray(const DenseSparseArray& spa) = default;
    DenseSparseArray(DenseSparseArray&& spa) noexcept = default;
    ~DenseSparseArray() = default;

    DenseSparseArray& operator=(const DenseSparseArray&) = default;
    DenseSparseArray& operator=(DenseSparseArray&&) noexcept = default;

    reference_type operator[](size_type idx) {
        if (hasComponent(idx))
            return _dense[_spar[PAGE(idx)][PAGE_INDEX(idx)].value()];
        else
            return std::nullopt;
    }

    const_reference_type operator[](size_type idx) const {
        if (hasComponent(idx))
            return _dense[_spar[PAGE(idx)][PAGE_INDEX(idx)].value()];
        else
            return std::nullopt;
    }

    template <typename ...Args>
    reference_type emplace_at(size_type e, Args&&... args) {
        if (PAGE(e) >= _spar.size())
            _spar.resize(PAGE(e) + 1);
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.emplace_back(std::forward<Args>(args)...);
        _dense_to_entity.push_back(e);
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

    reference_type insert_at(size_type e, Component&& cmpt) {
        if (PAGE(e) >= _spar.size()) {
            _spar.resize(PAGE(e) + 1);
        }
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.push_back(std::forward<Component>(cmpt));
        _dense_to_entity.push_back(e);
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

    reference_type insert_at(size_type e, const Component& cmpt) {
        if (PAGE(e) >= _spar.size()) {
            _spar.resize(PAGE(e) + 1);
        }
        _spar[PAGE(e)].insert_at(PAGE_INDEX(e), _dense.size());
        _dense.push_back(cmpt);
        _dense_to_entity.push_back(e);
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

    void erase(size_type e) {
        if (!hasComponent(e))
            return;
        std::size_t del_index = _spar[PAGE(e)][PAGE_INDEX(e)].value();
        size_type back_e = _dense_to_entity.back();
        if (del_index != _dense.size() - 1) {
            std::swap(_dense[del_index], _dense.back());
            std::swap(_dense_to_entity[del_index], _dense_to_entity.back());
            _spar[PAGE(back_e)][PAGE_INDEX(back_e)] = del_index;
        }
        _spar[PAGE(e)][PAGE_INDEX(e)] = std::nullopt;
        _dense.pop_back();
        _dense_to_entity.pop_back();
    }

    size_type get_index(const_reference_type value) const {
        auto i = std::find(_dense.begin(), _dense.end(), value);

        if (hasComponent(_dense_to_entity[i]))
            return _dense_to_entity[i];
        return _spar.size() * MAX_PAGE_SIZE + _spar[_spar.size() - 1].size();
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

    Component& getComponent(size_type e) {
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

    const Component& getComponent(size_type e) const {
        return _dense[_spar[PAGE(e)][PAGE_INDEX(e)].value()];
    }

 private:
    std::vector<Component> _dense;
    std::vector<size_type> _dense_to_entity;
    std::vector<SparseArray<size_type>> _spar;

    bool hasComponent(size_type e) {
        return (PAGE(e) < _spar.size() &&
            PAGE_INDEX(e) <_spar[PAGE(e)].size() &&
            _spar[PAGE(e)][PAGE_INDEX(e)].has_value());
    }
};

}  // namespace dng
