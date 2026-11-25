#pragma once

// std
#include <concepts>
#include <utility>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/slots/having.hpp>
#include <fluxmt/sql/query/slots/where.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    // A condition joined with the others of its slot by AND: WHERE, HAVING.
    FLUXMT_EXPORT Clause make_condition_clause(SlotRef slot, Expr condition);
}


namespace flux::impl::clauses {
    // Expr only: 'where("flag")' must not silently bind a string.
    template<std::same_as<Expr> T>
    Clause where(T condition) {
        return make_condition_clause(&slots::where, std::move(condition));
    }

    template<std::same_as<Expr> T>
    Clause having(T condition) {
        return make_condition_clause(&slots::having, std::move(condition));
    }
}
