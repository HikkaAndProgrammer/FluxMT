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
    inline constexpr function_rule_t now_as_keyword {
        .name      = "now",
        .max_arity = 0,
        .spelling  = { .text = "CURRENT_TIMESTAMP", .call = false },
    };

    inline constexpr function_rule_t greatest_as_max {
        .name      = "greatest",
        .min_arity = 2,
        .spelling  = { .text = "max" },
    };

    inline constexpr function_rule_t least_as_min {
        .name      = "least",
        .min_arity = 2,
        .spelling  = { .text = "min" },
    };

    inline constexpr function_rules_t sqlite_functions {
        now_as_keyword,
        greatest_as_max,
        least_as_min,
    };

    inline constexpr syntax_t sqlite_syntax {
        .cast      = &syntax::cast_function,
        .ilike     = &syntax::ilike_via_lower,
        .any       = &syntax::any_as_in_list,
        .excluded  = &syntax::excluded_table,
        .functions = sqlite_functions,
    };


    class FLUXMT_EXPORT sqlite_dialect final : public SqlDialect {
    public:
        sqlite_dialect() noexcept;

        std::string_view name() const noexcept override;
        std::string placeholder(size_t index) const override;
        bool is_reserved(std::string_view word) const noexcept override;
        slot_info_view layout(kind_ref kind) const noexcept override;
    };


    FLUXMT_EXPORT const sqlite_dialect& dialect() noexcept;

    FLUXMT_EXPORT std::span<const std::string_view> reserved_words() noexcept;
}
