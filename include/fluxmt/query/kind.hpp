#pragma once

// std
#include <string_view>


namespace flux::impl {
    struct statement_kind_t {
        std::string_view name;
    };
    using kind_ref = const statement_kind_t*;


    constexpr bool same_kind(kind_ref lhs, kind_ref rhs) noexcept {
        return lhs && rhs && lhs->name == rhs->name;
    }
}
