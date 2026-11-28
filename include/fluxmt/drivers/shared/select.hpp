#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/column.hpp>
#include <fluxmt/sql/slots/distinct.hpp>
#include <fluxmt/sql/slots/from.hpp>
#include <fluxmt/sql/slots/group.hpp>
#include <fluxmt/sql/slots/having.hpp>
#include <fluxmt/sql/slots/head.hpp>
#include <fluxmt/sql/slots/join.hpp>
#include <fluxmt/sql/slots/limit.hpp>
#include <fluxmt/sql/slots/offset.hpp>
#include <fluxmt/sql/slots/order.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t select_keyword {
        .slot     = &slots::select_head,
        .fallback = "SELECT",
    };

    inline constexpr slot_info_t distinct {
        .slot = &slots::distinct,
    };

    inline constexpr slot_info_t select_columns {
        .slot      = &slots::column,
        .prefix    = " ",
        .separator = ", ",
        .fallback  = "*",
    };

    inline constexpr slot_info_t from {
        .slot      = &slots::from,
        .prefix    = "\nFROM ",
        .separator = ", ",
    };

    inline constexpr slot_info_t join {
        .slot = &slots::join,
    };

    inline constexpr slot_info_t group {
        .slot      = &slots::group,
        .prefix    = "\nGROUP BY ",
        .separator = ", ",
    };

    inline constexpr slot_info_t having {
        .slot      = &slots::having,
        .prefix    = "\nHAVING ",
        .separator = " AND ",
    };

    inline constexpr slot_info_t order {
        .slot      = &slots::order,
        .prefix    = "\nORDER BY ",
        .separator = ", ",
    };

    inline constexpr slot_info_t limit {
        .slot   = &slots::limit,
        .prefix = "\nLIMIT ",
    };

    inline constexpr slot_info_t offset {
        .slot   = &slots::offset,
        .prefix = "\nOFFSET ",
    };
}
