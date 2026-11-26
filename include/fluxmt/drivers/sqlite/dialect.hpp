#pragma once

// std
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/syntax.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::sqlite {
    constexpr function_spelling_t spell_function(std::string_view name, size_t arity) noexcept {
        if (name == "now") {
            if (arity != 0)
                return { .supported = false, .reason = "now() takes no arguments" };

            return { .name = "CURRENT_TIMESTAMP", .call = false };
        }

        if (name == "greatest" || name == "least") {
            if (arity < 2)
                return { .supported = false, .reason = "needs two arguments or more, max() and min() of one are aggregates" };

            return { .name = name == "greatest" ? "max" : "min" };
        }

        return {};
    }

    inline constexpr syntax_t sqlite_syntax {
        .cast     = &syntax::cast_function,
        .ilike    = &syntax::ilike_via_lower,
        .any      = &syntax::any_as_in_list,
        .excluded = &syntax::excluded_table,
        .function = &spell_function,
    };


    class FLUXMT_EXPORT sqlite_dialect final : public Dialect {
    public:
        sqlite_dialect() noexcept;

        std::string_view name() const noexcept override;
        std::string placeholder(size_t index) const override;
        bool is_reserved(std::string_view word) const noexcept override;
        LayoutView layout(KindRef kind) const noexcept override;
    };


    FLUXMT_EXPORT const sqlite_dialect& dialect() noexcept;

    FLUXMT_EXPORT std::span<const std::string_view> reserved_words() noexcept;
}
