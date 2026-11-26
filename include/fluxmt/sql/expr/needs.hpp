#pragma once

// std
#include <cstddef>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    enum class ESyntax : ers::u8 {
        Cast,
        Ilike,
        Any,
        Excluded,
    };

    struct fn_need_t {
        static constexpr size_t max_name = 23;

        char name[max_name + 1] = {};
        ers::u8 arity = 0;

        constexpr bool operator==(const fn_need_t&) const noexcept = default;

        constexpr std::string_view view() const noexcept { return name; }
    };

    struct needs_t {
        static constexpr size_t max_fns = 4;

        ers::u8 syntax = 0;
        ers::u8 count = 0;

        bool partial = false;

        fn_need_t fns[max_fns] = {};

        constexpr bool operator==(const needs_t&) const noexcept = default;
    };

    inline constexpr needs_t no_needs {};
    inline constexpr needs_t unknown_needs { .partial = true };
}


// Implementation --------------------------------------------------------------

namespace flux::impl {
    [[nodiscard]]
    constexpr needs_t need_syntax(ESyntax what) noexcept {
        needs_t out;
        out.syntax = static_cast<ers::u8>(1u << static_cast<unsigned>(what));

        return out;
    }

    [[nodiscard]]
    constexpr needs_t need_function(std::string_view name, size_t arity) noexcept {
        needs_t out;

        if (name.size() > fn_need_t::max_name || arity > 255) {
            out.partial = true;

            return out;
        }

        for (size_t i = 0; i < name.size(); ++i)
            out.fns[0].name[i] = name[i];

        out.fns[0].arity = static_cast<ers::u8>(arity);
        out.count = 1;

        return out;
    }

    [[nodiscard]]
    constexpr bool has_syntax(const needs_t& needs, ESyntax what) noexcept {
        return (needs.syntax & (1u << static_cast<unsigned>(what))) != 0;
    }

    [[nodiscard]]
    constexpr needs_t operator|(const needs_t& lhs, const needs_t& rhs) noexcept {
        needs_t out = lhs;

        out.syntax = static_cast<ers::u8>(out.syntax | rhs.syntax);
        out.partial = lhs.partial || rhs.partial;

        for (size_t i = 0; i < rhs.count; ++i) {
            bool known = false;

            for (size_t j = 0; j < out.count; ++j) {
                if (out.fns[j] == rhs.fns[i])
                    known = true;
            }

            if (known)
                continue;

            if (out.count == needs_t::max_fns) {
                out.partial = true;
                continue;
            }

            out.fns[out.count++] = rhs.fns[i];
        }

        return out;
    }
}
