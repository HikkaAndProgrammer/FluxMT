#pragma once

// std
#include <concepts>
#include <type_traits>

// fluxmt
#include <fluxmt/query/slot_clause.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/needs.hpp>


namespace flux::impl {
    template<slot_ref S, needs_t N = unknown_needs>
    using SqlClause = SlotClause<S, false, N>;

    template<slot_ref S, needs_t N = unknown_needs>
    using SqlClauses = SlotClause<S, true, N>;


    template<typename C>
    constexpr needs_t clause_needs() noexcept {
        if constexpr (std::same_as<std::remove_cv_t<decltype(C::meta)>, needs_t>)
            return C::meta;
        else
            return unknown_needs;
    }

    template<typename C>
    constexpr slot_ref clause_slot() noexcept {
        return C::slot;
    }


    template<typename T>
    constexpr needs_t column_need() noexcept {
        using U = std::remove_cvref_t<T>;

        if constexpr (std::same_as<U, aliased_t> || std::same_as<U, column_expr_t>)
            return unknown_needs;
        else
            return needs_of_one<U>();
    }

    template<typename... Ts>
    constexpr needs_t column_needs() noexcept {
        needs_t out;
        ((out = out | column_need<Ts>()), ...);

        return out;
    }
}
