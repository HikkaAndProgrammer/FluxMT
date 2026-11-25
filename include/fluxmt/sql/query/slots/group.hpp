#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot group {
        .name  = "flux.group",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding group {
        .slot      = &slots::group,
        .prefix    = "\nGROUP BY ",
        .separator = ", ",
    };
}
