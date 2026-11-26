#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot distinct {
        .name  = "flux.distinct",
        .arity = EArity::Single,
    };
}
