#pragma once

// std
#include <cstddef>
#include <initializer_list>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/expr/needs.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::syntax {
    FLUXMT_EXPORT ers::Status cast_postfix(Context& ctx, const Expr& operand, std::string_view type);

    FLUXMT_EXPORT ers::Status cast_function(Context& ctx, const Expr& operand, std::string_view type);

    FLUXMT_EXPORT ers::Status ilike_operator(Context& ctx, const Expr& lhs, const Expr& rhs);

    FLUXMT_EXPORT ers::Status ilike_via_lower(Context& ctx, const Expr& lhs, const Expr& rhs);

    FLUXMT_EXPORT ers::Status any_native(Context& ctx, const Expr& operand, const Expr& array);

    FLUXMT_EXPORT ers::Status any_as_in_list(Context& ctx, const Expr& operand, const Expr& array);

    FLUXMT_EXPORT ers::Status excluded_table(Context& ctx, std::string_view column);
}


// Compile-time check ----------------------------------------------------------

namespace flux::impl {
    [[nodiscard]]
    constexpr bool has_strategy(const syntax_t& syntax, ESyntax what) noexcept {
        switch (what) {
            case ESyntax::Cast:     return syntax.cast != nullptr;
            case ESyntax::Ilike:    return syntax.ilike != nullptr;
            case ESyntax::Any:      return syntax.any != nullptr;
            case ESyntax::Excluded: return syntax.excluded != nullptr;
        }

        return false;
    }

    [[nodiscard]]
    constexpr needs_t missing(const syntax_t& syntax, const needs_t& needs) noexcept {
        needs_t out;

        for (const auto what : { ESyntax::Cast, ESyntax::Ilike, ESyntax::Any, ESyntax::Excluded }) {
            if (has_syntax(needs, what) && !has_strategy(syntax, what))
                out = out | need_syntax(what);
        }

        if (!syntax.function)
            return out;

        for (size_t i = 0; i < needs.count; ++i) {
            const auto& fn = needs.fns[i];

            if (!syntax.function(fn.view(), fn.arity).supported)
                out = out | need_function(fn.view(), fn.arity);
        }

        return out;
    }

    [[nodiscard]]
    constexpr bool supports(const syntax_t& syntax, const needs_t& needs) noexcept {
        const auto lacking = missing(syntax, needs);

        return lacking.syntax == 0 && lacking.count == 0;
    }
}
