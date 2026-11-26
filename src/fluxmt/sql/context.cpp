#include "fluxmt/sql/context.hpp"

// std
#include <memory>
#include <ranges>
#include <string>

// fluxmt
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/identifier.hpp>


// Implementation --------------------------------------------------------------

// SqlDialect

std::unique_ptr<flux::impl::Context> flux::impl::SqlDialect::make_context() const {
    return std::make_unique<SqlContext>(*this);
}


// SqlContext

flux::impl::SqlContext::SqlContext(const SqlDialect& dialect) noexcept :
    Context(dialect) {
}

const flux::impl::SqlDialect& flux::impl::SqlContext::dialect() const noexcept {
    return static_cast<const SqlDialect&>(Context::dialect());
}

ers::Status flux::impl::SqlContext::append_identifier(std::string_view name) {
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

        if (is_plain_identifier(part) && !dialect().is_reserved(part))
            out += part;
        else
            out += quote_identifier(part);
    }

    text += out;

    return ers::ok;
}
