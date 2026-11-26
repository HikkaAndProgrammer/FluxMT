#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot from {
        .name  = "flux.from",
        .arity = EArity::Multi,
    };
}
