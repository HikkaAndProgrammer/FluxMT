#include "fluxmt/query/context.hpp"

// std
#include <utility>


// Implementation --------------------------------------------------------------

// Member functions

flux::impl::Context::Context(const Dialect& dialect) noexcept :
    _dialect(&dialect) {
}


// Observers

const flux::impl::Dialect& flux::impl::Context::dialect() const noexcept {
    return *_dialect;
}

const std::vector<flux::Value>& flux::impl::Context::params() const noexcept {
    return _params;
}


// Modifiers

std::string flux::impl::Context::bind(Value value) {
    _params.push_back(std::move(value));
    return _dialect->placeholder(_params.size());
}

std::string flux::impl::Context::bind_once(const void* key, Value value) {
    if (auto it = _bound.find(key); it != _bound.end())
        return it->second;

    return _bound.emplace(key, bind(std::move(value)))
        .first->second;
}

flux::impl::statement_t flux::impl::Context::finish() && {
    return {
        .text   = std::move(text),
        .params = std::move(_params)
    };
}
