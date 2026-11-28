#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/clauses/with.hpp>
#include <fluxmt/sql/slots/with.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t with {
        .slot     = &slots::with,
        .renderer = &render_with,
    };

    inline constexpr slot_info_t set_union {
        .slot = &slots::set_union,
    };
}
