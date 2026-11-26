#include "fluxmt/sql/clauses/order.hpp"

// fluxmt
#include <fluxmt/sql/expr/functions.hpp>

// clauses
#include "order.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::order_by(column_expr_t what, EOrder order) {
    return make_clause<OrderClause>(std::move(what.expr), order);
}

flux::impl::Clause flux::impl::clauses::order_by_random() {
    return order_by(named_fn<"random"> {}());
}
