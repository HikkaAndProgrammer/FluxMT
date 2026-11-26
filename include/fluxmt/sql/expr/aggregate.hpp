#pragma once

// std
#include <concepts>
#include <string>
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
    template<ers::fixed_string Fn, typename... Args>
        requires (std::constructible_from<column_expr_t, Args> && ...)
    auto agg(Args&&... args) {
        return retype<needs_of<Args...>()>(make_aggregate(Fn.to_sv(), {
            .args = { column_expr_t(std::forward<Args>(args)).expr... }
        }));
    }

    template<ers::fixed_string Fn, typename A>
        requires std::constructible_from<column_expr_t, A>
    auto agg_distinct(A&& what) {
        return retype<needs_of<A>()>(make_aggregate(Fn.to_sv(), {
            .args     = { column_expr_t(std::forward<A>(what)).expr },
            .distinct = true
        }));
    }


    inline auto count_all() {
        return retype<no_needs>(make_aggregate("count", { .args = { make_const("*") } }));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto count_of(A&& what) {
        return agg<"count">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto count_distinct(A&& what) {
        return agg_distinct<"count">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto min_of(A&& what) {
        return agg<"min">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto max_of(A&& what) {
        return agg<"max">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto sum_of(A&& what) {
        return agg<"sum">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto avg_of(A&& what) {
        return agg<"avg">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto array_agg_of(A&& what) {
        return agg<"array_agg">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto bool_and_of(A&& what) {
        return agg<"bool_and">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto bool_or_of(A&& what) {
        return agg<"bool_or">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto stddev_of(A&& what) {
        return agg<"stddev">(std::forward<A>(what));
    }

    template<typename A>
        requires std::constructible_from<column_expr_t, A>
    auto variance_of(A&& what) {
        return agg<"variance">(std::forward<A>(what));
    }

    template<typename Y, typename X>
        requires (std::constructible_from<column_expr_t, Y> && std::constructible_from<column_expr_t, X>)
    auto corr_of(Y&& y, X&& x) {
        return agg<"corr">(std::forward<Y>(y), std::forward<X>(x));
    }

    template<typename A, IsExprLike D>
        requires std::constructible_from<column_expr_t, A>
    auto string_agg(A&& what, D&& delimiter) {
        return retype<needs_of<A, D>()>(make_aggregate("string_agg", {
            .args = { column_expr_t(std::forward<A>(what)).expr, Expr(std::forward<D>(delimiter)) }
        }));
    }


    FLUXMT_EXPORT Expr percentile_cont(double fraction, std::vector<column_expr_t> order_by);
    FLUXMT_EXPORT Expr percentile_disc(double fraction, std::vector<column_expr_t> order_by);
    FLUXMT_EXPORT Expr mode_of(std::vector<column_expr_t> order_by);

    FLUXMT_EXPORT Expr json_rows(std::string table, std::vector<column_expr_t> order_by = {});
}
