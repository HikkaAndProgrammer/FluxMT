#include "fluxmt/drivers/sqlite/dialect.hpp"

// std
#include <array>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql/kinds.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    // LIMIT -1: OFFSET needs a LIMIT
    constexpr slot_info_t offset {
        .slot        = &slots::offset,
        .prefix      = "\nOFFSET ",
        .lead_unless = &slots::limit,
        .lead        = "\nLIMIT -1",
    };

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
        offset,
        shared::set_union,
    };

    constexpr slot_info_t insert_layout[] = {
        shared::with,
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
        shared::conflict,
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
        { .kind = &kinds::select, .layout = select_layout },
        { .kind = &kinds::insert, .layout = insert_layout },
        { .kind = &kinds::update, .layout = update_layout },
        { .kind = &kinds::erase, .layout = delete_layout },
        { .kind = &kinds::exists, .layout = exists_layout },
    });
}


// Implementation --------------------------------------------------------------

slot_info_view sqlite::sqlite_dialect::layout(kind_ref kind) const noexcept {
    return find_layout(table, kind);
}
