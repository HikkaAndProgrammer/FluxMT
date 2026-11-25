#include "fluxmt/sql/query/clauses/bound.hpp"

// fluxmt
#include <fluxmt/sql/query/slots/limit.hpp>
#include <fluxmt/sql/query/slots/offset.hpp>

// clauses
#include "bound.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::with_limit(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::limit, amount);
}

flux::impl::Clause flux::impl::clauses::with_offset(ers::i64 amount) {
    return make_clause<BoundClause>(&slots::offset, amount);
}
