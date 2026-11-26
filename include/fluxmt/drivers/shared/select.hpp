#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/column.hpp>
#include <fluxmt/sql/query/slots/distinct.hpp>
#include <fluxmt/sql/query/slots/from.hpp>
#include <fluxmt/sql/query/slots/group.hpp>
#include <fluxmt/sql/query/slots/having.hpp>
#include <fluxmt/sql/query/slots/head.hpp>
#include <fluxmt/sql/query/slots/join.hpp>
#include <fluxmt/sql/query/slots/limit.hpp>
#include <fluxmt/sql/query/slots/offset.hpp>
#include <fluxmt/sql/query/slots/order.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding select_keyword {
        .slot     = &slots::select_head,
        .fallback = "SELECT",
    };

    inline constexpr SlotBinding distinct {
        .slot = &slots::distinct,
    };

    inline constexpr SlotBinding select_columns {
        .slot      = &slots::column,
        .prefix    = " ",
        .separator = ", ",
        .fallback  = "*",
    };

    inline constexpr SlotBinding from {
        .slot      = &slots::from,
        .prefix    = "\nFROM ",
        .separator = ", ",
    };

    inline constexpr SlotBinding join {
        .slot = &slots::join,
    };

    inline constexpr SlotBinding group {
        .slot      = &slots::group,
        .prefix    = "\nGROUP BY ",
        .separator = ", ",
    };

    inline constexpr SlotBinding having {
        .slot      = &slots::having,
        .prefix    = "\nHAVING ",
        .separator = " AND ",
    };

    inline constexpr SlotBinding order {
        .slot      = &slots::order,
        .prefix    = "\nORDER BY ",
        .separator = ", ",
    };

    inline constexpr SlotBinding limit {
        .slot   = &slots::limit,
        .prefix = "\nLIMIT ",
    };

    inline constexpr SlotBinding offset {
        .slot   = &slots::offset,
        .prefix = "\nOFFSET ",
    };
}
