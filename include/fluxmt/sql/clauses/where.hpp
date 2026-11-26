#pragma once

// std
#include <concepts>
#include <utility>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/slots/having.hpp>
#include <fluxmt/sql/slots/where.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_condition_clause(slot_ref slot, Expr condition);
}


namespace flux::impl::clauses {
    template<std::derived_from<Expr> T>
    SqlClause<&slots::where, needs_of<T>()> where(T condition) {
        return SqlClause<&slots::where, needs_of<T>()>(make_condition_clause(&slots::where, std::move(condition)));
    }

    template<std::derived_from<Expr> T>
    SqlClause<&slots::having, needs_of<T>()> having(T condition) {
        return SqlClause<&slots::having, needs_of<T>()>(make_condition_clause(&slots::having, std::move(condition)));
    }
}
