#pragma once

// std
#include <span>
#include <string>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/query.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT ers::Status render_with(Context& ctx, std::span<const Clause* const> group);
}


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause with_cte(std::string name, Query definition, bool recursive = false);

    FLUXMT_EXPORT Clause union_with(Query other, bool all = false);
}
