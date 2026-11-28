#pragma once

// std
#include <concepts>
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/expr/aggregate.hpp>
#include <fluxmt/sql/expr/expr.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto array_agg_of(T&& what) {
        return clauses::agg<"array_agg">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto bool_and_of(T&& what) {
        return clauses::agg<"bool_and">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto bool_or_of(T&& what) {
        return clauses::agg<"bool_or">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto stddev_of(T&& what) {
        return clauses::agg<"stddev">(std::forward<T>(what));
    }

    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    auto variance_of(T&& what) {
        return clauses::agg<"variance">(std::forward<T>(what));
    }

    template<typename Y, typename X>
        requires (std::constructible_from<column_expr_t, Y> && std::constructible_from<column_expr_t, X>)
    auto corr_of(Y&& y, X&& x) {
        return clauses::agg<"corr">(std::forward<Y>(y), std::forward<X>(x));
    }

    template<typename T, IsExprLike D>
        requires std::constructible_from<column_expr_t, T>
    auto string_agg(T&& what, D&& delimiter) {
        return retype<needs_of<T, D>()>(make_aggregate("string_agg", {
            .args = { column_expr_t(std::forward<T>(what)).expr, Expr(std::forward<D>(delimiter)) }
        }));
    }


    FLUXMT_EXPORT Expr percentile_cont(double fraction, std::vector<column_expr_t> order_by);
    FLUXMT_EXPORT Expr percentile_disc(double fraction, std::vector<column_expr_t> order_by);
    FLUXMT_EXPORT Expr mode_of(std::vector<column_expr_t> order_by);

    FLUXMT_EXPORT Expr json_rows(std::string table, std::vector<column_expr_t> order_by = {});
}
