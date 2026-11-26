#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding returning {
        .slot      = &slots::returning,
        .prefix    = "\nRETURNING ",
        .separator = ", ",
    };
}
