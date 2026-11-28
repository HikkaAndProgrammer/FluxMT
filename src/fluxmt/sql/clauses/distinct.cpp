#include "fluxmt/sql/clauses/distinct.hpp"

// fluxmt
#include <fluxmt/sql/clauses/raw.hpp>
#include <fluxmt/sql/slots/distinct.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_distinct_clause() {
    return make_raw_clause(&slots::distinct, " DISTINCT", {});
}
