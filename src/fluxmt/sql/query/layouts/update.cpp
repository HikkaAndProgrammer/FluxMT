#include "fluxmt/sql/query/layouts/update.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>
#include <fluxmt/sql/query/slots/set.hpp>
#include <fluxmt/sql/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot update_slot {
        .name  = "flux.update",
        .arity = EArity::Single,
    };

    constexpr SlotBinding update_layout[] = {
        {
            .slot   = &update_slot,
            .prefix = "UPDATE "
        },
        bindings::set,
        bindings::where,
        bindings::returning,
    };
}


// Implementation --------------------------------------------------------------

Query layouts::update(std::string table) {
    Query q(update_layout);
    q |= clauses::identifier(&update_slot, std::move(table));

    return q;
}
