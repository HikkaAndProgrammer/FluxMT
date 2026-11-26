#include "fluxmt/sql/expr/aggregate.hpp"

// std
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// nodes
#include "nodes/aggregate/aggregate.hpp"


// Implementation --------------------------------------------------------------

flux::impl::Expr flux::impl::make_aggregate(std::string_view name, aggregate_t what) {
    return make_expr<nodes::AggregateNode>(name, std::move(what));
}

std::vector<flux::impl::Expr> flux::impl::to_exprs(std::vector<ColumnExpr> items) {
    std::vector<Expr> out;
    out.reserve(items.size());

    for (auto& it : items)
        out.push_back(std::move(it.expr));

    return out;
}


// Shortcuts

flux::impl::Expr flux::impl::clauses::percentile_cont(double fraction, std::vector<ColumnExpr> order_by) {
    return make_aggregate("percentile_cont", {
        .args         = { Expr(fraction) },
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::clauses::percentile_disc(double fraction, std::vector<ColumnExpr> order_by) {
    return make_aggregate("percentile_disc", {
        .args         = { Expr(fraction) },
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::clauses::mode_of(std::vector<ColumnExpr> order_by) {
    return make_aggregate("mode", {
        .within_group = to_exprs(std::move(order_by)),
        .ordered_set  = true,
    });
}

flux::impl::Expr flux::impl::clauses::json_rows(std::string table, std::vector<ColumnExpr> order_by) {
    return make_aggregate("json_agg", {
        .args     = { make_fn("row_to_json", { col(std::move(table)) }) },
        .order_by = to_exprs(std::move(order_by)),
    });
}
