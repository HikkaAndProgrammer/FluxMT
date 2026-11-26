#include "fluxmt/sql/context.hpp"

// std
#include <ranges>
#include <utility>

// fluxmt
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/identifier.hpp>


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

ers::Status flux::impl::Context::append_identifier(std::string_view name) {
    if (name.empty())
        return ers::make_error(ESqlError::InvalidIdentifier, "an empty name");


    std::string out;
    bool first = true;

    for (auto piece : std::views::split(name, '.')) {
        std::string_view part(piece.begin(), piece.end());

        if (part.empty() || part.contains('\0')) {
            return ers::make_error(ESqlError::InvalidIdentifier, "'{}'",
                name);
        }

        if (!first)
            out += '.';
        first = false;

        if (is_plain_identifier(part) && !_dialect->is_reserved(part))
            out += part;
        else
            out += quote_identifier(part);
    }

    query += out;


    return ers::ok;
}

flux::impl::statement_t flux::impl::Context::finish() && {
    return {
        .sql    = std::move(query),
        .params = std::move(_params)
    };
}
