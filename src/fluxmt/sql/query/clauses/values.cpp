#include "fluxmt/sql/query/clauses/values.hpp"

// clauses
#include "values.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_values_clause(std::vector<Expr> items) {
    return make_clause<ValuesClause>(std::move(items));
}
