#include "fluxmt/sql/query/clauses/distinct.hpp"

// fluxmt
#include <fluxmt/sql/query/clauses/raw.hpp>
#include <fluxmt/sql/query/slots/distinct.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::distinct() {
    return make_raw_clause(&slots::distinct, " DISTINCT", {});
}
