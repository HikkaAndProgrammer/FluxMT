#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t having {
        .name  = "flux.having",
        .arity = EArity::Multi,
    };
}
