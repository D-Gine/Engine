/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <expected>
    #include <optional>
    #include <functional>

/**
 * @brief std::expected wrapper that transparently holds a reference
 *
 * Inherits from std::expected<std::reference_wrapper<Object>, Error> and
 * overrides operator*, value(), and operator-> to unwrap the reference
 * automatically.
 *
 * @tparam Object The referenced object type
 * @tparam Error  The error type returned on failure
 *
 * Example usage:
 * @code{.cpp}
 * ExpectRef<DenseSparseArray<Position>, TypeNotRegistred> getArray() {
 *     if (!registered)
 *         return std::unexpected(TypeNotRegistred("Position"));
 *     return array;   // implicitly wraps the reference
 * }
 *
 * auto result = getArray();
 * if (result.has_value()) {
 *     result->addComponent(entity, data);   // arrow operator
 *     auto& arr = *result;                  // dereference operator
 *     auto& arr = result.value();           // value() method
 * } else {
 *     std::println("error: {}", result.error().what());
 * }
 * @endcode
 */
template <typename Object, typename Error>
class ExpectRef : public std::expected<std::reference_wrapper<Object>, Error> {
 private:
    using Base = std::expected<std::reference_wrapper<Object>, Error>;

 public:
    ExpectRef() = default;
    ExpectRef(std::unexpected<Error>&& val) : Base(std::move(val)) {}
    ExpectRef(const std::unexpected<Error>& val) : Base(val) {}
    ExpectRef(Object& obj) : Base(std::ref(obj)) {}

    Object& operator*() { return Base::value().get(); }
    const Object& operator*() const { return Base::value().get(); }

    Object& value() { return Base::value().get(); }
    const Object& value() const { return Base::value().get(); }

    Object* operator->() { return &(Base::value().get()); }
    const Object* operator->() const { return &(Base::value().get()); }
};

/**
 * @brief std::optional wrapper that transparently holds a reference
 *
 * Inherits from std::optional<std::reference_wrapper<Object>> and overrides
 * operator*, value(), and operator-> to unwrap the reference automatically.
 *
 * Use this when you only need "present / absent" semantics and don't need
 * an error value. For error details, use ExpectRef instead.
 *
 * @tparam Object The referenced object type
 *
 * Example usage:
 * @code{.cpp}
 * OptionalRef<std::string> findName(int id) {
 *     auto it = names.find(id);
 *     if (it == names.end())
 *         return std::nullopt;
 *     return it->second;   // implicitly wraps the reference
 * }
 *
 * auto result = findName(42);
 * if (result.has_value()) {
 *     std::string& name = *result;    // dereference operator
 *     result->clear();                // arrow operator
 * }
 * @endcode
 */
template <typename Object>
class OptionalRef : public std::optional<std::reference_wrapper<Object>> {
 private:
    using Base = std::optional<std::reference_wrapper<Object>>;

 public:
    OptionalRef() = default;
    OptionalRef(std::nullopt_t&& val) : Base(std::move(val)) {}
    OptionalRef(const std::nullopt_t& val) : Base(val) {}
    OptionalRef(Object& obj) : Base(std::ref(obj)) {}

    Object& operator*() { return Base::value().get(); }
    const Object& operator*() const { return Base::value().get(); }

    Object& value() { return Base::value().get(); }
    const Object& value() const { return Base::value().get(); }

    Object* operator->() { return &(Base::value().get()); }
    const Object* operator->() const { return &(Base::value().get()); }
};
