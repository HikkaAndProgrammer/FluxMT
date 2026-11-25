#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot column {
        .name  = "flux.column",
        .arity = EArity::Multi,
    };
}
