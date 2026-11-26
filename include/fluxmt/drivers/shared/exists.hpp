#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/head.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding exists_target {
        .slot   = &slots::exists_target,
        .prefix = "SELECT EXISTS (\nSELECT 1 FROM ",
    };

    inline constexpr SlotBinding exists_alias {
        .slot     = &slots::exists_alias,
        .prefix   = "\n) AS ",
        .fallback = "\"exists\"",
    };
}
