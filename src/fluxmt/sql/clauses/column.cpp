#include "fluxmt/sql/clauses/column.hpp"

// clauses
#include "column.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_column_clause(slot_ref slot, column_expr_t what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr));
}

flux::impl::Clause flux::impl::make_column_clause(slot_ref slot, aliased_t what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr), std::move(what.alias));
}
