#include "fluxmt/sql/identifier.hpp"

// std
#include <stdexcept>

// ers
#include <erslib/core/stdlib/ctype.hpp>


bool flux::impl::is_plain_identifier(std::string_view name) noexcept {
    if (name.empty() || !(ers::is_lower(name.front()) || name.front() == '_'))
        return false;

    for (char c : name.substr(1)) {
        if (!ers::is_lower(c) && !ers::is_digit(c) && c != '_')
            return false;
    }

    return true;
}

std::string flux::impl::quote_identifier(std::string_view name) {
    if (name.empty())
        throw std::invalid_argument("an identifier cannot be empty");


    std::string out;
    out.reserve(name.size() + 2);


    out += '"';

    for (char c : name) {
        if (c == '\0')
            throw std::invalid_argument("an identifier cannot contain NUL");

        if (c == '"')
            out += '"';

        out += c;
    }

    out += '"';


    return out;
}
