#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/conflict.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t conflict {
        .slot      = &slots::conflict,
        .prefix    = "\nON CONFLICT (",
        .separator = ", ",
        .suffix    = ")",
    };

    inline constexpr slot_info_t conflict_constraint {
        .slot   = &slots::conflict_constraint,
        .prefix = "\nON CONFLICT ON CONSTRAINT ",
    };

    inline constexpr slot_info_t conflict_action {
        .slot   = &slots::conflict_action,
        .prefix = "\n",
    };
}
