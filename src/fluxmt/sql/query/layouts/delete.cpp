#include "fluxmt/sql/query/layouts/delete.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>
#include <fluxmt/sql/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot delete_from_slot {
        .name  = "flux.delete_from",
        .arity = EArity::Single,
    };

    constexpr SlotBinding delete_layout[] = {
        {
            .slot   = &delete_from_slot,
            .prefix = "DELETE FROM "
        },
        bindings::where,
        bindings::returning,
    };
}


// Implementation --------------------------------------------------------------

Query layouts::delete_from(std::string table) {
    Query q(delete_layout);
    q |= clauses::identifier(&delete_from_slot, std::move(table));

    return q;
}
