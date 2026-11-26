#pragma once

// std
#include <cstddef>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/slots/set.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_assign_clause(std::string column, Expr value);

    FLUXMT_EXPORT Clause make_assign_null_clause(std::string column);

    FLUXMT_EXPORT ers::Status render_set_as_values(Context& ctx, std::span<const Clause* const> group);


    template<typename... Args, size_t... I>
    constexpr needs_t set_needs(std::index_sequence<I...>) noexcept {
        using pack = std::tuple<Args...>;

        return needs_of<std::tuple_element_t<2 * I + 1, pack>...>();
    }
}


namespace flux::impl::clauses {
    template<typename T>
    SqlClause<&slots::set, needs_of<T>()> assign(std::string column, T&& value) {
        return SqlClause<&slots::set, needs_of<T>()>(
            make_assign_clause(std::move(column), Expr(std::forward<T>(value))));
    }

    inline SqlClause<&slots::set, no_needs> assign_null(std::string column) {
        return SqlClause<&slots::set, no_needs>(make_assign_null_clause(std::move(column)));
    }


    template<typename... Args>
        requires (sizeof...(Args) >= 2 && sizeof...(Args) % 2 == 0)
    auto set(Args&&... args) {
        constexpr auto needs = set_needs<Args...>(std::make_index_sequence<sizeof...(Args) / 2> {});

        std::vector<Clause> out;
        out.reserve(sizeof...(Args) / 2);

        auto pairs = std::forward_as_tuple(std::forward<Args>(args)...);

        [&]<size_t... I>(std::index_sequence<I...>) {
            (out.push_back(make_assign_clause(
                std::string(std::get<2 * I>(pairs)),
                Expr(std::get<2 * I + 1>(std::move(pairs)))
            )), ...);
        }(std::make_index_sequence<sizeof...(Args) / 2> {});

        return SqlClauses<&slots::set, needs>(std::move(out));
    }
}
