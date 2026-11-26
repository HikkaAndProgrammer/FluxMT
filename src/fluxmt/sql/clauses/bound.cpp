#include "fluxmt/sql/clauses/bound.hpp"

// fluxmt
#include <fluxmt/sql/slots/limit.hpp>
#include <fluxmt/sql/slots/offset.hpp>

// clauses
#include "bound.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::with_limit(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

flux::impl::Clause flux::impl::clauses::with_offset(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
