#pragma once

// std
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/slot_clause.hpp>


namespace flux::impl::clauses {
    inline std::vector<Clause> when(bool condition, Clause clause) {
        std::vector<Clause> out;

        if (condition)
            out.push_back(std::move(clause));

        return out;
    }

    inline std::vector<Clause> when(bool condition, std::vector<Clause> clauses) {
        if (!condition)
            return {};

        return clauses;
    }

    template<slot_ref Slot, bool Many, auto Meta>
    SlotClause<Slot, true, Meta> when(bool condition, SlotClause<Slot, Many, Meta> clause) {
        if (!condition)
            return {};

        return SlotClause<Slot, true, Meta>(std::move(clause).take_all());
    }

    template<typename Fn>
        requires std::invocable<Fn>
    auto when(bool condition, Fn&& make) {
        using result_t = std::remove_cvref_t<std::invoke_result_t<Fn>>;

        if constexpr (is_slot_clause_v<result_t>) {
            using out_t = SlotClause<result_t::slot, true, result_t::meta>;

            if (!condition)
                return out_t {};

            return out_t(std::forward<Fn>(make)().take_all());
        } else if constexpr (std::same_as<result_t, std::vector<Clause>>) {
            if (!condition)
                return std::vector<Clause> {};

            return std::forward<Fn>(make)();
        } else {
            std::vector<Clause> out;

            if (condition)
                out.push_back(std::forward<Fn>(make)());

            return out;
        }
    }
}
