#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot set {
        .name  = "flux.set",
        .arity = EArity::Multi,
    };
}
