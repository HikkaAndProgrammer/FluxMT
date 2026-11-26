#pragma once

// std
#include <array>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/sql/kinds.hpp>


namespace flux::impl::sqlite {
    // LIMIT -1: OFFSET needs a LIMIT
    inline constexpr slot_info_t offset_with_lead {
        .slot        = &slots::offset,
        .prefix      = "\nOFFSET ",
        .lead_unless = &slots::limit,
        .lead        = "\nLIMIT -1",
    };

    inline constexpr slot_info_t select_layout[] = {
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
        offset_with_lead,
        shared::set_union,
    };

    inline constexpr slot_info_t insert_layout[] = {
        shared::with,
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
        shared::conflict,
        shared::conflict_action,
        shared::returning,
    };

    inline constexpr slot_info_t update_layout[] = {
        shared::with,
        shared::update_target,
        shared::set,
        shared::where,
        shared::returning,
    };

    inline constexpr slot_info_t delete_layout[] = {
        shared::with,
        shared::delete_from,
        shared::where,
        shared::returning,
    };

    inline constexpr slot_info_t exists_layout[] = {
        shared::with,
        shared::exists_target,
        shared::where,
        shared::exists_alias,
    };

    inline constexpr auto layout_table = std::to_array<layout_entry_t>({
        { .kind = &kinds::select, .layout = select_layout },
        { .kind = &kinds::insert, .layout = insert_layout },
        { .kind = &kinds::update, .layout = update_layout },
        { .kind = &kinds::erase,  .layout = delete_layout },
        { .kind = &kinds::exists, .layout = exists_layout },
    });
}
