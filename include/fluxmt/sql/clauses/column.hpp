#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/slots/column.hpp>

// export
#include <fluxmt/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    concept IsColumnItem = std::constructible_from<column_expr_t, T> || std::same_as<std::remove_cvref_t<T>, aliased_t>;

    template<typename R>
    concept IsColumnNamesRange = std::ranges::input_range<R>
        && std::constructible_from<std::string, std::ranges::range_reference_t<R>>;


    FLUXMT_EXPORT Clause make_column_clause(slot_ref slot, column_expr_t what);
    FLUXMT_EXPORT Clause make_column_clause(slot_ref slot, aliased_t what);

    template<IsColumnItem... Args>
    std::vector<Clause> make_columns(slot_ref slot, Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.push_back(make_column_clause(slot, std::forward<Args>(args))), ...);

        return out;
    }

    template<IsColumnNamesRange R>
    std::vector<Clause> make_columns(slot_ref slot, R&& names) {
        std::vector<Clause> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(make_column_clause(slot, std::string(std::forward<decltype(it)>(it))));

        return out;
    }
}


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    template<IsColumnItem T>
    SqlClause<&slots::column, column_need<T>()> column(T&& what) {
        return SqlClause<&slots::column, column_need<T>()>(make_column_clause(&slots::column, std::forward<T>(what)));
    }

    template<IsColumnItem... Args>
    SqlClauses<&slots::column, column_needs<Args...>()> columns(Args&&... args) {
        return SqlClauses<&slots::column, column_needs<Args...>()>(
            make_columns(&slots::column, std::forward<Args>(args)...));
    }

    template<IsColumnNamesRange R>
    SqlClauses<&slots::column, no_needs> columns(R&& names) {
        return SqlClauses<&slots::column, no_needs>(make_columns(&slots::column, std::forward<R>(names)));
    }
}
