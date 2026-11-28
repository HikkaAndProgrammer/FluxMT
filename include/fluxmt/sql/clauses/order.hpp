#pragma once

// std
#include <concepts>
#include <utility>

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/slots/order.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    enum class EOrder : ers::u8 { Asc, Desc };


    FLUXMT_EXPORT Clause make_order_clause(column_expr_t what, EOrder order);

    FLUXMT_EXPORT Clause make_order_random_clause();
}


namespace flux::impl::clauses {
    template<typename T>
        requires std::constructible_from<column_expr_t, T>
    SqlClause<&slots::order, column_need<T>()> order_by(T&& what, EOrder order = EOrder::Asc) {
        return SqlClause<&slots::order, column_need<T>()>(make_order_clause(column_expr_t(std::forward<T>(what)), order));
    }

    inline SqlClause<&slots::order, need_function("random", 0)> order_by_random() {
        return SqlClause<&slots::order, need_function("random", 0)>(make_order_random_clause());
    }
}
