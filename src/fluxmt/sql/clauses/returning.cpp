#include "fluxmt/sql/clauses/returning.hpp"

// fluxmt
#include <fluxmt/sql/clauses/raw.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::returning_all() {
    return make_raw_clause(&slots::returning, "*", {});
}
