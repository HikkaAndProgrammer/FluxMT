#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t distinct {
        .name  = "flux.distinct",
        .arity = EArity::Single,
    };
}
