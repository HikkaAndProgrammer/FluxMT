#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t with {
        .name  = "flux.with",
        .arity = EArity::Multi,
    };

    inline constexpr slot_t set_union {
        .name  = "flux.union",
        .arity = EArity::Multi,
    };
}
