/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <utility>
    #include <optional>
    #include <vector>
    #include <algorithm>

namespace dng {

/**
 * @brief Sparse Array implementation for the ECS
 *
 * Useful to manage a large amount of components associated with indexs as
 * entities in an Entity Component System type of architecture.
 * This class inherite from the std::vector and use std::optional type as
 * variables wich makes it easier to access
 *
 * std::vector<std::optional<Component>>
 *
 * @tparam Component Is the type of the component you are storing
 */
template<class Component>
class SparseArray {
 public :
    using value_type = std::optional<Component>;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using container_t = std::vector<value_type>;

    using size_type = typename container_t::size_type;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

 public:
    SparseArray() = default;
    explicit SparseArray(size_t size) : _data(size) {}
    SparseArray(const SparseArray& spa) = default;
    SparseArray(SparseArray&& spa) noexcept = default;
    ~SparseArray() = default;

    SparseArray& operator=(const SparseArray& spa) = default;
    SparseArray& operator=(SparseArray&& spa) noexcept = default;

    reference_type operator[](size_t idx) {
        return _data[idx];
    }
    const_reference_type operator[](size_t idx) const {
        return _data[idx];
    }

    iterator begin() {
        return _data.begin();
    }
    const_iterator begin() const {
        return _data.begin();
    }
    const_iterator cbegin() const {
        return _data.cbegin();
    }
    iterator end() {
        return _data.end();
    }
    const_iterator end() const {
        return _data.end();
    }
    const_iterator cend() const {
        return _data.cend();
    }
    size_type size() const {
        return _data.size();
    }

    template<typename... Args>
    reference_type emplace_at(size_type pos, Args&&... args) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(std::forward<Args>(args)...);
        return _data[pos];
    }

    reference_type insert_at(size_type pos, const Component& cmpt) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(cmpt);
        return _data[pos];
    }
    reference_type insert_at(size_type pos, Component&& cmpt) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(std::forward<Component>(cmpt));
        return _data[pos];
    }

    void erase(size_type pos) {
        _data[pos].reset();
    }

    size_type get_index(const value_type& idx) const {
        auto i = std::find(_data.begin(), _data.end(), idx);

        if (i == _data.end())
            return _data.size();
        return std::distance(_data.begin(), i);
    }

 private:
    container_t _data;
};

}  // namespace dng
