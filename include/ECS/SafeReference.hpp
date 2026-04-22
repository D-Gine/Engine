/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

    #include <expected>
    #include <optional>
    #include <functional>

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
