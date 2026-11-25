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
    // fn([DISTINCT] args [ORDER BY order_by]) [WITHIN GROUP (ORDER BY within_group)]
    struct aggregate_t {
        std::vector<Expr> args;
        std::vector<Expr> order_by;
        std::vector<Expr> within_group;
        bool distinct = false;
        bool ordered_set = false;
    };


    FLUXMT_EXPORT Expr make_aggregate(std::string_view name, aggregate_t what);

    FLUXMT_EXPORT std::vector<Expr> to_exprs(std::vector<ColumnExpr> items);
}


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    // For anything without a shorthand: agg<"regr_slope">("y", "x").
    template<ers::fixed_string Fn, typename... Args>
        requires (std::constructible_from<ColumnExpr, Args> && ...)
    Expr agg(Args&&... args) {
        return make_aggregate(Fn.to_sv(), {
            .args = { ColumnExpr(std::forward<Args>(args)).expr... }
        });
    }

    template<ers::fixed_string Fn>
    Expr agg_distinct(ColumnExpr what) {
        return make_aggregate(Fn.to_sv(), {
            .args     = { std::move(what.expr) },
            .distinct = true
        });
    }


    FLUXMT_EXPORT Expr count_all();
    FLUXMT_EXPORT Expr count_of(ColumnExpr what);
    FLUXMT_EXPORT Expr count_distinct(ColumnExpr what);

    FLUXMT_EXPORT Expr min_of(ColumnExpr what);
    FLUXMT_EXPORT Expr max_of(ColumnExpr what);
    FLUXMT_EXPORT Expr sum_of(ColumnExpr what);
    FLUXMT_EXPORT Expr avg_of(ColumnExpr what);

    FLUXMT_EXPORT Expr array_agg_of(ColumnExpr what);
    FLUXMT_EXPORT Expr bool_and_of(ColumnExpr what);
    FLUXMT_EXPORT Expr bool_or_of(ColumnExpr what);

    FLUXMT_EXPORT Expr stddev_of(ColumnExpr what);
    FLUXMT_EXPORT Expr variance_of(ColumnExpr what);

    FLUXMT_EXPORT Expr corr_of(ColumnExpr y, ColumnExpr x);

    // The delimiter is a value.
    FLUXMT_EXPORT Expr string_agg(ColumnExpr what, Expr delimiter);

    FLUXMT_EXPORT Expr percentile_cont(double fraction, std::vector<ColumnExpr> order_by);
    FLUXMT_EXPORT Expr percentile_disc(double fraction, std::vector<ColumnExpr> order_by);
    FLUXMT_EXPORT Expr mode_of(std::vector<ColumnExpr> order_by);

    // json_agg(row_to_json(<table>) ORDER BY ...)
    FLUXMT_EXPORT Expr json_rows(std::string table, std::vector<ColumnExpr> order_by = {});
}
