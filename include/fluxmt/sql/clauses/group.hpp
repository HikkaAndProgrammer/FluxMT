#pragma once

// std
#include <concepts>
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/column.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/group.hpp>


namespace flux::impl::clauses {
    template<typename... Args>
        requires (std::constructible_from<column_expr_t, Args> && ...)
    SqlClauses<&slots::group, column_needs<Args...>()> group_by(Args&&... args) {
        return SqlClauses<&slots::group, column_needs<Args...>()>(
            make_columns(&slots::group, std::forward<Args>(args)...));
    }

    template<ColumnNames R>
    SqlClauses<&slots::group, no_needs> group_by(R&& names) {
        return SqlClauses<&slots::group, no_needs>(make_columns(&slots::group, std::forward<R>(names)));
    }
}
