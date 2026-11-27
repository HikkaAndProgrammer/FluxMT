#pragma once

// std
#include <concepts>
#include <type_traits>

// fluxmt
#include <fluxmt/query/slot_clause.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/needs.hpp>


namespace flux::impl {
    template<slot_ref Slot, needs_t Needs = unknown_needs>
    using SqlClause = SlotClause<Slot, false, Needs>;

    template<slot_ref Slot, needs_t Needs = unknown_needs>
    using SqlClauses = SlotClause<Slot, true, Needs>;


    template<typename Tag>
    constexpr needs_t clause_needs() noexcept {
        if constexpr (std::same_as<std::remove_cv_t<decltype(Tag::meta)>, needs_t>)
            return Tag::meta;
        else
            return unknown_needs;
    }

    template<typename Tag>
    constexpr slot_ref clause_slot() noexcept {
        return Tag::slot;
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
