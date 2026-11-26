#include "fluxmt/sql/clauses/bound.hpp"

// fluxmt
#include <fluxmt/sql/slots/limit.hpp>
#include <fluxmt/sql/slots/offset.hpp>

// clauses
#include "bound.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_limit_clause(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

flux::impl::Clause flux::impl::make_offset_clause(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
