#include "fluxmt/sql/identifier.hpp"

// std
#include <stdexcept>
#include <ranges>

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

bool flux::impl::is_valid_name(std::string_view name) noexcept {
    if (name.empty())
        return false;

    for (auto piece : std::views::split(name, '.')) {
        std::string_view part(piece.begin(), piece.end());

        if (part.empty() || ers::is_digit(part.front()))
            return false;

        for (char c : part) {
            if (!ers::is_alnum(c) && c != '_')
                return false;
        }
    }

    return true;
}

bool flux::impl::is_type_name(std::string_view name) noexcept {
    while (name.ends_with("[]"))
        name.remove_suffix(2);

    return is_valid_name(name);
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
