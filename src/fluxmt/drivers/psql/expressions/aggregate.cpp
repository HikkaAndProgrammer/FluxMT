#include "fluxmt/drivers/psql/expressions/aggregate.hpp"

// std
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/expr/factories.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Expr flux::impl::psql::percentile_cont(double fraction, std::vector<column_expr_t> order_by) {
    return make_aggregate("percentile_cont", {
        .args         = { Expr(fraction) },
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::psql::percentile_disc(double fraction, std::vector<column_expr_t> order_by) {
    return make_aggregate("percentile_disc", {
        .args         = { Expr(fraction) },
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::psql::mode_of(std::vector<column_expr_t> order_by) {
    return make_aggregate("mode", {
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::psql::json_rows(std::string table, std::vector<column_expr_t> order_by) {
    return make_aggregate("json_agg", {
        .args     = { make_fn("row_to_json", { clauses::col(std::move(table)) }) },
        .order_by = to_exprs(std::move(order_by)),
    });
}
