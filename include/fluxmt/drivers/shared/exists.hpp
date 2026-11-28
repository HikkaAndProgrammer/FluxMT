#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/head.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t exists_target {
        .slot   = &slots::exists_target,
        .prefix = "SELECT EXISTS (\nSELECT 1 FROM ",
    };

    inline constexpr slot_info_t exists_alias {
        .slot     = &slots::exists_alias,
        .prefix   = "\n) AS ",
        .fallback = "\"exists\"",
    };
}
