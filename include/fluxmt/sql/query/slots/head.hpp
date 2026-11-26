#pragma once

// fluxmt
#include <fluxmt/sql/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot select_head {
        .name  = "flux.select_kw",
        .arity = EArity::Single,
    };

    inline constexpr Slot insert_target {
        .name  = "flux.insert_into",
        .arity = EArity::Single,
    };

    inline constexpr Slot update_target {
        .name  = "flux.update",
        .arity = EArity::Single,
    };

    inline constexpr Slot delete_target {
        .name  = "flux.delete_from",
        .arity = EArity::Single,
    };

    inline constexpr Slot exists_target {
        .name  = "flux.exists_target",
        .arity = EArity::Single,
    };

    inline constexpr Slot exists_alias {
        .name  = "flux.exists_alias",
        .arity = EArity::Single,
    };
}
