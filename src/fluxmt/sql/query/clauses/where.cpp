#include "fluxmt/sql/query/clauses/where.hpp"

// clauses
#include "condition.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_condition_clause(SlotRef slot, Expr condition) {
    return make_clause<ConditionClause>(slot, std::move(condition));
}
