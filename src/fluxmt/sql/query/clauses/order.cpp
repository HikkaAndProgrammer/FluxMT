#include "fluxmt/sql/query/clauses/order.hpp"

// fluxmt
#include <fluxmt/sql/expr/functions.hpp>

// clauses
#include "order.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::order_by(ColumnExpr what, EOrder order) {
    return make_clause<OrderClause>(std::move(what.expr), order);
}

flux::impl::Clause flux::impl::clauses::order_by_random() {
    return order_by(fn_t<"random"> {}());
}
