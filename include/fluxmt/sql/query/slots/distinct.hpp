#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot distinct {
        .name  = "flux.distinct",
        .arity = EArity::Single,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding distinct {
        .slot = &slots::distinct,
    };
}
