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


// Shortcuts

flux::impl::Clause flux::impl::clauses::column(column_expr_t what) {
    return make_column_clause(&slots::column, std::move(what));
}

flux::impl::Clause flux::impl::clauses::column(aliased_t what) {
    return make_column_clause(&slots::column, std::move(what));
}
