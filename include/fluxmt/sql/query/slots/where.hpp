#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot where {
        .name  = "flux.where",
        .arity = EArity::Multi,
    };
}
