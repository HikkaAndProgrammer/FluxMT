#pragma once

// std
#include <string_view>


namespace flux::impl {
    // What a statement is. A tag like Slot, so anyone can add their own kind and a dialect maps it to a layout.
    struct StatementKind {
        std::string_view name;
    };

    using KindRef = const StatementKind*;


    [[nodiscard]]
    constexpr bool same_kind(KindRef lhs, KindRef rhs) noexcept {
        return lhs && rhs && lhs->name == rhs->name;
    }
}


namespace flux::impl::kinds {
    inline constexpr StatementKind select { .name = "flux.select" };
    inline constexpr StatementKind insert { .name = "flux.insert" };
    inline constexpr StatementKind update { .name = "flux.update" };
    inline constexpr StatementKind erase  { .name = "flux.delete" };
    inline constexpr StatementKind exists { .name = "flux.exists" };
}
