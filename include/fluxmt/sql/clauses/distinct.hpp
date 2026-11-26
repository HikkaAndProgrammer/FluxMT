#pragma once

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/distinct.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_distinct_clause();
}


namespace flux::impl::clauses {
    inline SqlClause<&slots::distinct, no_needs> distinct() {
        return SqlClause<&slots::distinct, no_needs>(make_distinct_clause());
    }
}
