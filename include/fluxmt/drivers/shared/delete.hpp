#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/head.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding delete_from {
        .slot   = &slots::delete_target,
        .prefix = "DELETE FROM ",
    };
}
