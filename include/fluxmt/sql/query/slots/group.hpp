#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot group {
        .name  = "flux.group",
        .arity = EArity::Multi,
    };
}
