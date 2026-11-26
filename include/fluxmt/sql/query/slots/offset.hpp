#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot offset {
        .name  = "flux.offset",
        .arity = EArity::Single,
    };
}
