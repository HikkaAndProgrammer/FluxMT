#pragma once

// std
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_values_clause(std::vector<Expr> items);
}


namespace flux::impl::clauses {
    // A value is bound, an Expr is rendered: values(url, now()).
    template<typename... Args>
    Clause values(Args&&... args) {
        std::vector<Expr> items;
        items.reserve(sizeof...(Args));
        (items.push_back(Expr(std::forward<Args>(args))), ...);

        return make_values_clause(std::move(items));
    }
}
