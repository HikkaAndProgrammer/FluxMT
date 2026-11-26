#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t offset {
        .name  = "flux.offset",
        .arity = EArity::Single,
    };
}
