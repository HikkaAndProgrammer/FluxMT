#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot returning {
        .name  = "flux.returning",
        .arity = EArity::Multi,
    };
}
