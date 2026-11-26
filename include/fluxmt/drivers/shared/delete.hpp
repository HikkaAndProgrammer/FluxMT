#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/head.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t delete_from {
        .slot   = &slots::delete_target,
        .prefix = "DELETE FROM ",
    };
}
