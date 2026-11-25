#include "fluxmt/sql/query/clauses/returning.hpp"

// fluxmt
#include <fluxmt/sql/query/clauses/raw.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::returning_all() {
    return make_raw_clause(&slots::returning, "*", {});
}
