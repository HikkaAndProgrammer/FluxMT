#include "fluxmt/sql/clauses/distinct.hpp"

// fluxmt
#include <fluxmt/sql/clauses/raw.hpp>
#include <fluxmt/sql/slots/distinct.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::distinct() {
    return make_raw_clause(&slots::distinct, " DISTINCT", {});
}
