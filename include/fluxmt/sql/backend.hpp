#pragma once

// std
#include <concepts>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/query/kind.hpp>
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/expr/needs.hpp>
#include <fluxmt/sql/syntax.hpp>


namespace flux::impl {
    template<typename B>
    concept SqlBackend = Supports<B, EFeature::Sql> && requires(kind_ref kind) {
        { backend_traits<B>::dialect() } -> std::convertible_to<const SqlDialect&>;
        { backend_traits<B>::syntax } -> std::convertible_to<const syntax_t&>;
        { backend_traits<B>::layout(kind) } -> std::same_as<slot_info_view>;
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

    template<SqlBackend B>
    consteval slot_info_view layout_of(kind_ref kind) noexcept {
        return backend_traits<B>::layout(kind);
    }

    template<SqlBackend B>
    consteval const syntax_t& syntax_of() noexcept {
        return backend_traits<B>::syntax;
    }


    template<typename B, kind_ref K>
    concept HasKind = SqlBackend<B> && !layout_of<B>(K).empty();

    template<typename B, kind_ref K, slot_ref S>
    concept HasSlot = SqlBackend<B> && layout_has(layout_of<B>(K), S);

    template<typename B, needs_t N>
    concept SupportsNeeds = SqlBackend<B> && supports(syntax_of<B>(), N);

    template<typename B, kind_ref K, typename C>
    concept AcceptsClause = HasSlot<B, K, C::slot> && SupportsNeeds<B, clause_needs<C>()>;
}
