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

std::vector<flux::impl::Expr> flux::impl::to_exprs(std::vector<column_expr_t> items) {
    std::vector<Expr> out;
    out.reserve(items.size());

    for (auto& it : items)
        out.push_back(std::move(it.expr));

    return out;
}
