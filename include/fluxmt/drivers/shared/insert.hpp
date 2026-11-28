#pragma once

// fluxmt
#include <fluxmt/sql/clauses/assign.hpp>
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/column.hpp>
#include <fluxmt/sql/slots/head.hpp>
#include <fluxmt/sql/slots/set.hpp>
#include <fluxmt/sql/slots/values.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t insert_into {
        .slot   = &slots::insert_target,
        .prefix = "INSERT INTO ",
    };

    inline constexpr slot_ref set_as_values_excludes[] = { &slots::column, &slots::values };

    inline constexpr slot_info_t set_as_values {
        .slot     = &slots::set,
        .renderer = &render_set_as_values,
        .excludes = set_as_values_excludes,
    };

    inline constexpr slot_info_t insert_columns {
        .slot      = &slots::column,
        .prefix    = " (",
        .separator = ", ",
        .suffix    = ")",
    };

    inline constexpr slot_info_t values {
        .slot      = &slots::values,
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")",
    };
}
