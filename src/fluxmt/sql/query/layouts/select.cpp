#include "fluxmt/sql/query/layouts/select.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/column.hpp>
#include <fluxmt/sql/query/slots/distinct.hpp>
#include <fluxmt/sql/query/slots/from.hpp>
#include <fluxmt/sql/query/slots/group.hpp>
#include <fluxmt/sql/query/slots/having.hpp>
#include <fluxmt/sql/query/slots/join.hpp>
#include <fluxmt/sql/query/slots/limit.hpp>
#include <fluxmt/sql/query/slots/offset.hpp>
#include <fluxmt/sql/query/slots/order.hpp>
#include <fluxmt/sql/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot select_kw {
        .name  = "flux.select_kw",
        .arity = EArity::Single,
    };

    constexpr SlotBinding select_layout[] = {
        {
            .slot     = &select_kw,
            .fallback = "SELECT",
        },
        bindings::distinct,
        {
            .slot      = &slots::column,
            .prefix    = " ",
            .separator = ", ",
            .fallback  = "*",
        },
        bindings::from,
        bindings::join,
        bindings::where,
        bindings::group,
        bindings::having,
        bindings::order,
        bindings::limit,
        bindings::offset,
    };
}


// Implementation --------------------------------------------------------------

Query layouts::select_from(std::string table, std::string alias) {
    Query q(select_layout);
    q |= clauses::from(std::move(table), std::move(alias));

    return q;
}

Query layouts::select() {
    return Query(select_layout);
}
