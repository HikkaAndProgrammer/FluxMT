#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/slots/head.hpp>
#include <fluxmt/sql/slots/set.hpp>


namespace flux::impl::shared {
    inline constexpr slot_info_t update_target {
        .slot   = &slots::update_target,
        .prefix = "UPDATE ",
    };

    inline constexpr slot_info_t set {
        .slot      = &slots::set,
        .prefix    = "\nSET ",
        .separator = ", ",
    };
}
