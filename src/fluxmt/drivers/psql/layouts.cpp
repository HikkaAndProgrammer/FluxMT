#include "fluxmt/drivers/psql/dialect.hpp"

// std
#include <array>

// fluxmt
#include <fluxmt/drivers/shared.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    constexpr SlotBinding select_layout[] = {
        shared::select_keyword,
        shared::distinct,
        shared::select_columns,
        shared::from,
        shared::join,
        shared::where,
        shared::group,
        shared::having,
        shared::order,
        shared::limit,
        shared::offset,
    };

    constexpr SlotBinding insert_layout[] = {
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
        shared::conflict,
        shared::conflict_constraint,
        shared::conflict_action,
        shared::returning,
    };

    constexpr SlotBinding update_layout[] = {
        shared::update_target,
        shared::set,
        shared::where,
        shared::returning,
    };

    constexpr SlotBinding delete_layout[] = {
        shared::delete_from,
        shared::where,
        shared::returning,
    };

    constexpr SlotBinding exists_layout[] = {
        shared::exists_target,
        shared::where,
        shared::exists_alias,
    };

    constexpr auto table = std::to_array<layout_entry_t>({
        { &kinds::select, select_layout },
        { &kinds::insert, insert_layout },
        { &kinds::update, update_layout },
        { &kinds::erase,  delete_layout },
        { &kinds::exists, exists_layout },
    });
}


// Implementation --------------------------------------------------------------

LayoutView psql::postgres_dialect::layout(KindRef kind) const noexcept {
    return find_layout(table, kind);
}
