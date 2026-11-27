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
    template<typename Backend>
    concept IsSqlBackend = SupportsFeature<Backend, EFeature::Sql> && requires {
        { backend_traits<Backend>::dialect() } -> std::convertible_to<const SqlDialect&>;
        { backend_traits<Backend>::syntax } -> std::convertible_to<const syntax_t&>;
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

    template<IsSqlBackend Backend>
    consteval const syntax_t& syntax_of() noexcept {
        return backend_traits<Backend>::syntax;
    }


    template<typename Backend, const auto& Layout, slot_ref Slot>
    concept HasSlot = IsSqlBackend<Backend>
        && layout_has(slot_info_view(Layout), Slot);

    template<typename Backend, needs_t Needs>
    concept SupportsNeeds = IsSqlBackend<Backend>
        && supports(syntax_of<Backend>(), Needs);

    template<typename Backend, const auto& Layout, typename Tag>
    concept AcceptsClause = HasSlot<Backend, Layout, Tag::slot>
        && SupportsNeeds<Backend, clause_needs<Tag>()>;
}
