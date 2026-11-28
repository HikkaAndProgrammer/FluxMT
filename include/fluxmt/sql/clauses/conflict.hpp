#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/conflict.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_conflict_update_clause(std::vector<std::string> columns);

    FLUXMT_EXPORT Clause make_do_nothing_clause();
}


namespace flux::impl::clauses {
    inline SqlClause<&slots::conflict, no_needs> on_conflict(std::string column) {
        return SqlClause<&slots::conflict, no_needs>(identifier(&slots::conflict, std::move(column)));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 2 && (std::convertible_to<Args, std::string> && ...))
    SqlClauses<&slots::conflict, no_needs> on_conflict(Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.push_back(identifier(&slots::conflict, std::forward<Args>(args))), ...);

        return SqlClauses<&slots::conflict, no_needs>(std::move(out));
    }

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    SqlClauses<&slots::conflict, no_needs> on_conflict(R&& names) {
        std::vector<Clause> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(identifier(&slots::conflict, std::string(std::forward<decltype(it)>(it))));

        return SqlClauses<&slots::conflict, no_needs>(std::move(out));
    }


    inline SqlClause<&slots::conflict_constraint, no_needs> on_conflict_constraint(std::string name) {
        return SqlClause<&slots::conflict_constraint, no_needs>(
            identifier(&slots::conflict_constraint, std::move(name)));
    }


    inline SqlClause<&slots::conflict_action, no_needs> do_nothing() {
        return SqlClause<&slots::conflict_action, no_needs>(make_do_nothing_clause());
    }

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    SqlClause<&slots::conflict_action, need_syntax(ESyntax::Excluded)> do_update(R&& names) {
        std::vector<std::string> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(std::string(std::forward<decltype(it)>(it)));

        return SqlClause<&slots::conflict_action, need_syntax(ESyntax::Excluded)>(
            make_conflict_update_clause(std::move(out)));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 1 && (std::convertible_to<Args, std::string> && ...))
    SqlClause<&slots::conflict_action, need_syntax(ESyntax::Excluded)> do_update(Args&&... args) {
        std::vector<std::string> out;
        out.reserve(sizeof...(Args));
        (out.push_back(std::forward<Args>(args)), ...);

        return SqlClause<&slots::conflict_action, need_syntax(ESyntax::Excluded)>(
            make_conflict_update_clause(std::move(out)));
    }
}
