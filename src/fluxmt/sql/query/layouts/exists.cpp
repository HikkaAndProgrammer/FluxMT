#include "fluxmt/sql/query/layouts/exists.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot exists_target {
        .name  = "flux.exists_target",
        .arity = EArity::Single,
    };

    inline constexpr Slot exists_alias {
        .name  = "flux.exists_alias",
        .arity = EArity::Single,
    };

    constexpr SlotBinding exists_layout[] = {
        {
            .slot   = &exists_target,
            .prefix = "SELECT EXISTS (\nSELECT 1 FROM "
        },
        bindings::where,
        {
            .slot     = &exists_alias,
            .prefix   = "\n) AS ",
            .fallback = "\"exists\""
        },
    };
}


// Implementation --------------------------------------------------------------

Query layouts::exists_in(std::string table) {
    Query q(exists_layout);
    q |= clauses::identifier(&exists_target, std::move(table));

    return q;
}
