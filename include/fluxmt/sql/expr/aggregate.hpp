#pragma once

// std
#include <concepts>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>

// export
#include <fluxmt/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    struct aggregate_t {
        std::vector<Expr> args;
        std::vector<Expr> order_by;
        std::vector<Expr> within_group;
        bool distinct = false;
        bool ordered_set = false;
    };


    FLUXMT_EXPORT Expr make_aggregate(std::string_view name, aggregate_t what);

    FLUXMT_EXPORT std::vector<Expr> to_exprs(std::vector<column_expr_t> items);
}


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    template<ers::fixed_string Name, typename... Args>
        requires (std::constructible_from<column_expr_t, Args> && ...)
    auto agg(Args&&... args) {
        return retype<needs_of<Args...>()>(make_aggregate(Name.to_sv(), {
            .args = { column_expr_t(std::forward<Args>(args)).expr... }
        }));
    }

    template<ers::fixed_string Name, typename T>
        requires std::constructible_from<column_expr_t, T>
    auto agg_distinct(T&& what) {
        return retype<needs_of<T>()>(make_aggregate(Name.to_sv(), {
            .args     = { column_expr_t(std::forward<T>(what)).expr },
            .distinct = true
        }));
    }


    inline auto count_all() {
        return retype<no_needs>(make_aggregate("count", { .args = { make_const("*") } }));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto count_of(T&& what) {
        return agg<"count">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto count_distinct(T&& what) {
        return agg_distinct<"count">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto min_of(T&& what) {
        return agg<"min">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto max_of(T&& what) {
        return agg<"max">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto sum_of(T&& what) {
        return agg<"sum">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto avg_of(T&& what) {
        return agg<"avg">(std::forward<T>(what));
    }
}
