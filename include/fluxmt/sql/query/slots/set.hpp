#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot set {
        .name  = "flux.set",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding set {
        .slot      = &slots::set,
        .prefix    = "\nSET ",
        .separator = ", ",
    };
}
