#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>
#include <fluxmt/sql/query/slots/head.hpp>
#include <fluxmt/sql/query/slots/set.hpp>


namespace flux::impl::shared {
    inline constexpr SlotBinding update_target {
        .slot   = &slots::update_target,
        .prefix = "UPDATE ",
    };

    inline constexpr SlotBinding set {
        .slot      = &slots::set,
        .prefix    = "\nSET ",
        .separator = ", ",
    };
}
