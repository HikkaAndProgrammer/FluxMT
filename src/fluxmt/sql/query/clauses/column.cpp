#include "fluxmt/sql/query/clauses/column.hpp"

// clauses
#include "column.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_column_clause(SlotRef slot, ColumnExpr what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr));
}

flux::impl::Clause flux::impl::make_column_clause(SlotRef slot, Aliased what) {
    return make_clause<ColumnClause>(slot, std::move(what.expr), std::move(what.alias));
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::column(ColumnExpr what) {
    return make_column_clause(&slots::column, std::move(what));
}

flux::impl::Clause flux::impl::clauses::column(Aliased what) {
    return make_column_clause(&slots::column, std::move(what));
}
