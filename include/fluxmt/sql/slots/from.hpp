#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t from {
        .name  = "flux.from",
        .arity = EArity::Multi,
    };
}
