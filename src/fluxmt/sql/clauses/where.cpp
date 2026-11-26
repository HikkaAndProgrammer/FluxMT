#include "fluxmt/sql/clauses/where.hpp"

// clauses
#include "condition.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_condition_clause(slot_ref slot, Expr condition) {
    return make_clause<ConditionClause>(slot, std::move(condition));
}
