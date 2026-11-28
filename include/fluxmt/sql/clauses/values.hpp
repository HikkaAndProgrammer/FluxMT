#pragma once

// std
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/slots/values.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_values_clause(std::vector<Expr> items);
}


namespace flux::impl::clauses {
    template<typename... Args>
    SqlClause<&slots::values, needs_of<Args...>()> values(Args&&... args) {
        std::vector<Expr> items;
        items.reserve(sizeof...(Args));
        (items.push_back(Expr(std::forward<Args>(args))), ...);

        return SqlClause<&slots::values, needs_of<Args...>()>(make_values_clause(std::move(items)));
    }
}
