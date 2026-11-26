#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t order {
        .name  = "flux.order",
        .arity = EArity::Multi,
    };
}
