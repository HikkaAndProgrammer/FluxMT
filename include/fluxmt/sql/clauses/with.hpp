#pragma once

// std
#include <span>
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/query.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/with.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT ers::Status render_with(Context& ctx, std::span<const Clause* const> group);

    FLUXMT_EXPORT Clause make_cte_clause(std::string name, Query definition, bool recursive);

    FLUXMT_EXPORT Clause make_union_clause(Query other, bool all);
}


namespace flux::impl::clauses {
    inline SqlClause<&slots::with> with_cte(std::string name, Query definition, bool recursive = false) {
        return SqlClause<&slots::with>(make_cte_clause(std::move(name), std::move(definition), recursive));
    }

    inline SqlClause<&slots::set_union> union_with(Query other, bool all = false) {
        return SqlClause<&slots::set_union>(make_union_clause(std::move(other), all));
    }
}
