/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <tuple>
    #include <utility>
    #include <cstddef>
    #include <iterator>
    #include <type_traits>
    #include <functional>

    #include "ECS/DenseSparseArray.hpp"

namespace dng {

template <typename T>
struct unwrap_refwrapper {
    using type = T;
};

template <typename T>
struct unwrap_refwrapper<std::reference_wrapper<T>> {
    using type = T;
};

template <typename T>
using unwrap_refwrapper_t = typename unwrap_refwrapper<std::decay_t<T>>::type;

template <class... Containers>
class Zipper {
 public:
    template <class... Cs>
    class ZipperIterator {
     public:
        template <class Container>
        using unwrapped_t = unwrap_refwrapper_t<Container>;

        template <class Container>
        using component_t = typename unwrapped_t<Container>::value_type;

        using value_type = std::tuple<component_t<Cs>&...>;
        using reference = value_type;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        ZipperIterator(std::tuple<Cs...> containers, size_t page, size_t idx = 0) :
            _currents(containers), _page(page), _idx(idx), _is_end(false) {
            if (!_is_end && !all_set(_seq)) {
                incr_all();
            }
        }

        ZipperIterator& operator++() {
            incr_all();
            return *this;
        }
        ZipperIterator operator++(int) {
            auto prev = *this;
            ++(*this);
            return prev;
        }

        value_type operator*() {
            return to_value(_seq);
        }
        value_type operator->() {
            return to_value(_seq);
        }

        friend bool operator==(const ZipperIterator& lhs,
            const ZipperIterator& rhs) {
                return lhs._is_end == rhs._is_end &&
                    (lhs._is_end ||
                    (lhs._page == rhs._page && lhs._idx == rhs._idx));
        }
        friend bool operator!=(const ZipperIterator& lhs,
            const ZipperIterator& rhs) {
            return !(lhs == rhs);
        }

        size_t get_page() const {
            return _page;
        }

        size_t get_index() const {
            return _idx;
        }

     private:
        template <size_t I>
        auto& get_container() {
            auto& container = std::get<I>(_currents);
            if constexpr (std::is_same_v<std::decay_t<decltype(container)>,
                std::reference_wrapper<unwrapped_t<decltype(container)>>>) {
                return container.get();
            } else {
                return container;
            }
        }

        void incr_all() {
            auto& fcontainer = get_container<0>();
            auto& fspar = fcontainer.getSpar();

            ++_idx;
            while (_page < fspar.size()) {
                while (_idx < fspar[_page].size()) {
                    if (fspar[_page][_idx].has_value() && all_set(_seq)) {
                        return;
                    }
                    ++_idx;
                }
                ++_page;
                _idx = 0;
            }
            _is_end = true;
        }

        template <size_t... Is>
        bool all_set(std::index_sequence<Is...>) {
            return (has_component<Is>() && ...);
        }

        template <size_t... Is>
        value_type to_value(std::index_sequence<Is...>) {
            return std::tie(get_component<Is>()...);
        }

        template <size_t I>
        bool has_component() {
            auto& container = get_container<I>();
            auto& spar = container.getSpar();

            return _page < spar.size() &&
                    _idx < spar[_page].size() &&
                    spar[_page][_idx].has_value();
        }

        template <size_t I>
        auto& get_component() {
            auto& container = get_container<I>();
            auto& spar = container.getSpar();
            size_t dense_idx = spar[_page][_idx].value();
            return container.getComponentDense(dense_idx);
        }

        std::tuple<Cs...> _currents;
        size_t _page;
        size_t _idx;
        bool _is_end;
        static constexpr std::index_sequence_for<Cs...> _seq{};
    };

    using iterator = ZipperIterator<Containers...>;

    explicit Zipper(Containers... cs) :
        _currents(std::make_tuple(cs...)) {}

    iterator begin() {
        return iterator(_currents, 0, 0);
    }
    iterator end() {
        auto& first_container = [this]() -> auto& {
            auto& c = std::get<0>(_currents);
            if constexpr (std::is_same_v<std::decay_t<decltype(c)>,
                std::reference_wrapper<unwrap_refwrapper_t<decltype(c)>>>) {
                return c.get();
            } else {
                return c;
            }
        }();
        size_t max_page = first_container.getSpar().size();
        return iterator(_currents, max_page, 0);
    }

 private:
    std::tuple<Containers...> _currents;
    size_t _size;
};

template <class... Containers>
class IndexedZipper {
 public:
    class Iterator {
     public:
        using inner_iterator = typename Zipper<Containers...>::iterator;
        using value_type = std::tuple<size_t,
            typename inner_iterator::value_type>;
        using reference = value_type;

        explicit Iterator(inner_iterator it) : _it(it) {}

        Iterator& operator++() {
            ++_it;
            return *this;
        }
        Iterator operator++(int) {
            auto prev = *this;
            ++(*this);
            return prev;
        }
        auto operator*() {
            return std::tuple_cat(std::make_tuple(
                MAX_PAGE_SIZE * _it.get_page() + _it.get_index()), *_it);
        }
        bool operator==(Iterator const& other) const {
            return _it == other._it;
        }
        bool operator!=(Iterator const& other) const {
            return !(*this == other);
        }

     private:
        inner_iterator _it;
    };

    explicit IndexedZipper(Containers... cs) : _zipper(cs...) {}

    Iterator begin() {
        return Iterator(_zipper.begin());
    }

    Iterator end() {
        return Iterator(_zipper.end());
    }

 private:
    Zipper<Containers...> _zipper;
};

}  // namespace dng
