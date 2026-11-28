#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t group {
        .name  = "flux.group",
        .arity = EArity::Multi,
    };
}
