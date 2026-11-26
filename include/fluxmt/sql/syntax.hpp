#pragma once

// std
#include <cstddef>
#include <initializer_list>
#include <span>
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
    template<size_t N>
    struct function_rules_t {
        function_rule_t rules[N];

        constexpr operator std::span<const function_rule_t>() const noexcept { return rules; }
    };

    template<typename... R>
    function_rules_t(R...) -> function_rules_t<sizeof...(R)>;

    template<size_t A, size_t B>
    constexpr function_rules_t<A + B> operator+(const function_rules_t<A>& lhs, const function_rules_t<B>& rhs) noexcept {
        function_rules_t<A + B> out {};

        for (size_t i = 0; i < A; ++i)
            out.rules[i] = lhs.rules[i];

        for (size_t i = 0; i < B; ++i)
            out.rules[A + i] = rhs.rules[i];

        return out;
    }

    constexpr syntax_t with_functions(syntax_t base, std::span<const function_rule_t> functions) noexcept {
        base.functions = functions;

        return base;
    }

    constexpr function_spelling_t spelling_of(const syntax_t& syntax, std::string_view name, size_t arity) noexcept {
        bool named = false;

        for (const auto& rule : syntax.functions) {
            if (rule.name != name)
                continue;

            named = true;

            if (arity >= rule.min_arity && arity <= rule.max_arity)
                return rule.spelling;
        }

        if (named) {
            return {
                .supported = false,
                .reason    = "no spelling for this number of arguments"
            };
        }

        if (syntax.function)
            return syntax.function(name, arity);

        return {};
    }


    constexpr bool has_strategy(const syntax_t& syntax, ESyntax what) noexcept {
        switch (what) {
            case ESyntax::Cast:     return syntax.cast != nullptr;
            case ESyntax::Ilike:    return syntax.ilike != nullptr;
            case ESyntax::Any:      return syntax.any != nullptr;
            case ESyntax::Excluded: return syntax.excluded != nullptr;
        }

        return false;
    }

    constexpr needs_t missing(const syntax_t& syntax, const needs_t& needs) noexcept {
        needs_t out;

        for (const auto what : { ESyntax::Cast, ESyntax::Ilike, ESyntax::Any, ESyntax::Excluded }) {
            if (has_syntax(needs, what) && !has_strategy(syntax, what))
                out = out | need_syntax(what);
        }

        for (size_t i = 0; i < needs.count; ++i) {
            const auto& fn = needs.fns[i];

            if (!spelling_of(syntax, fn.view(), fn.arity).supported)
                out = out | need_function(fn.view(), fn.arity);
        }

        return out;
    }

    constexpr bool supports(const syntax_t& syntax, const needs_t& needs) noexcept {
        const auto lacking = missing(syntax, needs);
        return lacking.syntax == 0 && lacking.count == 0;
    }
}
