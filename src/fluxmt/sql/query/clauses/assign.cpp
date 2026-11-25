#include "fluxmt/sql/query/clauses/assign.hpp"

// clauses
#include "assign.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_assign_clause(std::string column, Expr value) {
    return make_clause<AssignClause>(std::move(column), std::move(value));
}


// Shortcuts

flux::impl::Clause flux::impl::clauses::assign_null(std::string column) {
    return make_assign_clause(std::move(column), null());
}
