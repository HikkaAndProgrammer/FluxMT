#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t column {
        .name  = "flux.column",
        .arity = EArity::Multi,
    };
}
