#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot order {
        .name  = "flux.order",
        .arity = EArity::Multi,
    };
}
