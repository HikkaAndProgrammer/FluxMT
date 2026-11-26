#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/slots/column.hpp>

// export
#include <fluxmt/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    concept ColumnItem = std::constructible_from<ColumnExpr, T> || std::same_as<std::remove_cvref_t<T>, Aliased>;

    template<typename R>
    concept ColumnNames = std::ranges::input_range<R>
        && std::constructible_from<std::string, std::ranges::range_reference_t<R>>;


    FLUXMT_EXPORT Clause make_column_clause(SlotRef slot, ColumnExpr what);
    FLUXMT_EXPORT Clause make_column_clause(SlotRef slot, Aliased what);

    template<ColumnItem... Args>
    std::vector<Clause> make_columns(SlotRef slot, Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.push_back(make_column_clause(slot, std::forward<Args>(args))), ...);

        return out;
    }

    template<ColumnNames R>
    std::vector<Clause> make_columns(SlotRef slot, R&& names) {
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
    FLUXMT_EXPORT Clause column(ColumnExpr what);
    FLUXMT_EXPORT Clause column(Aliased what);

    template<ColumnItem... Args>
    std::vector<Clause> columns(Args&&... args) {
        return make_columns(&slots::column, std::forward<Args>(args)...);
    }

    template<ColumnNames R>
    std::vector<Clause> columns(R&& names) {
        return make_columns(&slots::column, std::forward<R>(names));
    }
}
