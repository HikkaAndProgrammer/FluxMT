#include "fluxmt/sql/clauses/returning.hpp"

// fluxmt
#include <fluxmt/sql/clauses/raw.hpp>


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_returning_all_clause() {
    return make_raw_clause(&slots::returning, "*", {});
}
