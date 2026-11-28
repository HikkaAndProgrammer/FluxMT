#include "fluxmt/sql/clauses/order.hpp"

// fluxmt
#include <fluxmt/sql/expr/functions.hpp>

// clauses
#include "order.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_order_clause(column_expr_t what, EOrder order) {
    return make_clause<OrderClause>(std::move(what.expr), order);
}

flux::impl::Clause flux::impl::make_order_random_clause() {
    return make_order_clause(named_fn<"random"> {}(), EOrder::Asc);
}
