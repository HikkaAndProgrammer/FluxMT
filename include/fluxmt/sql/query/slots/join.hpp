#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot join {
        .name  = "flux.join",
        .arity = EArity::Multi,
    };
}
