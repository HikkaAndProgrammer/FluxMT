#pragma once

// std
#include <concepts>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/query/clauses/column.hpp>
#include <fluxmt/sql/query/slots/group.hpp>


namespace flux::impl::clauses {
    template<typename... Args>
        requires (std::constructible_from<ColumnExpr, Args> && ...)
    std::vector<Clause> group_by(Args&&... args) {
        return make_columns(&slots::group, std::forward<Args>(args)...);
    }

    template<ColumnNames R>
    std::vector<Clause> group_by(R&& names) {
        return make_columns(&slots::group, std::forward<R>(names));
    }
}
