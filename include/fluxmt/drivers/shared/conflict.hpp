#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding conflict {
        .slot      = &slots::conflict,
        .prefix    = "\nON CONFLICT (",
        .separator = ", ",
        .suffix    = ")",
    };

    inline constexpr SlotBinding conflict_constraint {
        .slot   = &slots::conflict_constraint,
        .prefix = "\nON CONFLICT ON CONSTRAINT ",
    };

    inline constexpr SlotBinding conflict_action {
        .slot   = &slots::conflict_action,
        .prefix = "\n",
    };
}
