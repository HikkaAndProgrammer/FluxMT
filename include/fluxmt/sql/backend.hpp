#pragma once

// std
#include <concepts>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/expr/needs.hpp>
#include <fluxmt/sql/syntax.hpp>


namespace flux::impl {
    template<typename B>
    concept IsSqlBackend = SupportsFeature<B, EFeature::Sql> && requires {
        { backend_traits<B>::dialect() } -> std::convertible_to<const SqlDialect&>;
        { backend_traits<B>::syntax } -> std::convertible_to<const syntax_t&>;
    };
}


namespace flux::impl {
    constexpr bool layout_has(slot_info_view layout, slot_ref slot) noexcept {
        for (const auto& it : layout) {
            if (same_slot(it.slot, slot))
                return true;
        }

        return false;
    }

    template<IsSqlBackend B>
    consteval const syntax_t& syntax_of() noexcept {
        return backend_traits<B>::syntax;
    }


    template<typename B, const auto& L, slot_ref S>
    concept HasSlot = IsSqlBackend<B> && layout_has(slot_info_view(L), S);

    template<typename B, needs_t N>
    concept SupportsNeeds = IsSqlBackend<B> && supports(syntax_of<B>(), N);

    template<typename B, const auto& L, typename C>
    concept AcceptsClause = HasSlot<B, L, C::slot> && SupportsNeeds<B, clause_needs<C>()>;
}
