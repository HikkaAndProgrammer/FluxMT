#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot conflict {
        .name  = "flux.conflict",
        .arity = EArity::Multi,
    };

    inline constexpr Slot conflict_constraint {
        .name  = "flux.conflict_constraint",
        .arity = EArity::Single,
    };

    inline constexpr Slot conflict_action {
        .name  = "flux.conflict_action",
        .arity = EArity::Single,
    };
}
