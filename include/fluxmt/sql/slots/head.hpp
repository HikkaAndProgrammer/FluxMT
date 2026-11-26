#pragma once

// fluxmt
#include <fluxmt/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr slot_t select_head {
        .name  = "flux.select_kw",
        .arity = EArity::Single,
    };

    inline constexpr slot_t insert_target {
        .name  = "flux.insert_into",
        .arity = EArity::Single,
    };

    inline constexpr slot_t update_target {
        .name  = "flux.update",
        .arity = EArity::Single,
    };

    inline constexpr slot_t delete_target {
        .name  = "flux.delete_from",
        .arity = EArity::Single,
    };

    inline constexpr slot_t exists_target {
        .name  = "flux.exists_target",
        .arity = EArity::Single,
    };

    inline constexpr slot_t exists_alias {
        .name  = "flux.exists_alias",
        .arity = EArity::Single,
    };
}
