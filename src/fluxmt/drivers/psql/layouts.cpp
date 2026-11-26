#include "fluxmt/drivers/psql/dialect.hpp"

// std
#include <array>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql/kinds.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    constexpr slot_info_t select_layout[] = {
        shared::with,
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
        shared::set_union,
    };

    constexpr slot_info_t insert_layout[] = {
        shared::with,
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
        shared::conflict,
        shared::conflict_constraint,
        shared::conflict_action,
        shared::returning,
    };

    constexpr slot_info_t update_layout[] = {
        shared::with,
        shared::update_target,
        shared::set,
        shared::where,
        shared::returning,
    };

    constexpr slot_info_t delete_layout[] = {
        shared::with,
        shared::delete_from,
        shared::where,
        shared::returning,
    };

    constexpr slot_info_t exists_layout[] = {
        shared::with,
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

slot_info_view psql::postgres_dialect::layout(kind_ref kind) const noexcept {
    return find_layout(table, kind);
}
