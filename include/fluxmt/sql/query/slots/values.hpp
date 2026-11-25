#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot values {
        .name  = "flux.values",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding values {
        .slot      = &slots::values,
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")",
    };
}
