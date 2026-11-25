#pragma once

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/sql/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause with_limit(ers::i64 amount);
    FLUXMT_EXPORT Clause with_offset(ers::i64 amount);
}
