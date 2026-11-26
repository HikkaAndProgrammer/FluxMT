#include "fluxmt/sql/clauses/join.hpp"

// clauses
#include "join.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_join_clause(std::string table, std::string alias, Expr on) {
    return make_clause<JoinClause>(std::move(table), std::move(alias), std::move(on));
}
