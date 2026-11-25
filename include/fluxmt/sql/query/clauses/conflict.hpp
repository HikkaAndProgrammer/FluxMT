#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    // The whole action: "DO UPDATE SET a = excluded.a, b = excluded.b"
    FLUXMT_EXPORT Clause make_conflict_update_clause(std::vector<std::string> columns);
}


namespace flux::impl::clauses {
    inline Clause on_conflict(std::string column) {
        return identifier(&slots::conflict, std::move(column));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 2 && (std::convertible_to<Args, std::string> && ...))
    std::vector<Clause> on_conflict(Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.push_back(identifier(&slots::conflict, std::forward<Args>(args))), ...);

        return out;
    }

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    std::vector<Clause> on_conflict(R&& names) {
        std::vector<Clause> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(identifier(&slots::conflict, std::string(std::forward<decltype(it)>(it))));

        return out;
    }


    FLUXMT_EXPORT Clause on_conflict_constraint(std::string name);


    FLUXMT_EXPORT Clause do_nothing();

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    Clause do_update(R&& names) {
        std::vector<std::string> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(std::string(std::forward<decltype(it)>(it)));

        return make_conflict_update_clause(std::move(out));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 1 && (std::convertible_to<Args, std::string> && ...))
    Clause do_update(Args&&... args) {
        std::vector<std::string> out;
        out.reserve(sizeof...(Args));
        (out.push_back(std::forward<Args>(args)), ...);

        return make_conflict_update_clause(std::move(out));
    }
}
