#pragma once

// fluxmt
#include <fluxmt/sql/query/clauses/assign.hpp>
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/column.hpp>
#include <fluxmt/sql/query/slots/head.hpp>
#include <fluxmt/sql/query/slots/set.hpp>
#include <fluxmt/sql/query/slots/values.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding insert_into {
        .slot   = &slots::insert_target,
        .prefix = "INSERT INTO ",
    };

    inline constexpr SlotRef set_as_values_excludes[] = { &slots::column, &slots::values };

    inline constexpr SlotBinding set_as_values {
        .slot     = &slots::set,
        .renderer = &render_set_as_values,
        .excludes = set_as_values_excludes,
    };

    inline constexpr SlotBinding insert_columns {
        .slot      = &slots::column,
        .prefix    = " (",
        .separator = ", ",
        .suffix    = ")",
    };

    inline constexpr SlotBinding values {
        .slot      = &slots::values,
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")",
    };
}
