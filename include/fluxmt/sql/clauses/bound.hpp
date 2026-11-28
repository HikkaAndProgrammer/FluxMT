#pragma once

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/limit.hpp>
#include <fluxmt/sql/slots/offset.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_limit_clause(ers::i64 amount);
    FLUXMT_EXPORT Clause make_offset_clause(ers::i64 amount);
}


namespace flux::impl::clauses {
    inline SqlClause<&slots::limit, no_needs> with_limit(ers::i64 amount) {
        return SqlClause<&slots::limit, no_needs>(make_limit_clause(amount));
    }

    inline SqlClause<&slots::offset, no_needs> with_offset(ers::i64 amount) {
        return SqlClause<&slots::offset, no_needs>(make_offset_clause(amount));
    }
}
