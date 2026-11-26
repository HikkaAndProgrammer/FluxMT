#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot having {
        .name  = "flux.having",
        .arity = EArity::Multi,
    };
}
