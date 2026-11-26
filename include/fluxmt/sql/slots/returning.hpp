#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t returning {
        .name  = "flux.returning",
        .arity = EArity::Multi,
    };
}
