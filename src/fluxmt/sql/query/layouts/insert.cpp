#include "fluxmt/sql/query/layouts/insert.hpp"

// std
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/slots/column.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>
#include <fluxmt/sql/query/slots/set.hpp>
#include <fluxmt/sql/query/slots/values.hpp>

// clauses
#include "../clauses/assign.inl"


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot insert_into_slot {
        .name  = "flux.insert_into",
        .arity = EArity::Single,
    };


    // SET of an INSERT is written as "(columns) VALUES (values)".
    ers::Status render_insert_set(Context& ctx, std::span<const Clause* const> group) {
        std::vector<const AssignClause*> assigns;
        assigns.reserve(group.size());

        for (const auto* clause : group) {
            const auto* assign = dynamic_cast<const AssignClause*>(&**clause);

            if (!assign) {
                ERS_TRY((*clause)->render(ctx));

                return ers::make_error(ESqlError::InvalidSetItem, "SET of INSERT");
            }

            assigns.push_back(assign);
        }

        ctx.query += " (";

        for (size_t i = 0; i < assigns.size(); ++i) {
            if (i != 0)
                ctx.query += ", ";

            ERS_TRY(assigns[i]->render_column(ctx));
        }

        ctx.query += ")\nVALUES (";

        for (size_t i = 0; i < assigns.size(); ++i) {
            if (i != 0)
                ctx.query += ", ";

            ERS_TRY(assigns[i]->render_value(ctx));
        }

        ctx.query += ")";

        return ers::ok;
    }


    constexpr SlotRef set_excludes[] = { &slots::column, &slots::values };

    constexpr SlotBinding insert_layout[] = {
        {
            .slot   = &insert_into_slot,
            .prefix = "INSERT INTO "
        },
        {
            .slot     = &slots::set,
            .renderer = render_insert_set,
            .excludes = set_excludes
        },
        {
            .slot      = &slots::column,
            .prefix    = " (",
            .separator = ", ",
            .suffix    = ")"
        },
        bindings::values,
        bindings::conflict,
        bindings::conflict_constraint,
        bindings::conflict_action,
        bindings::returning,
    };
}


// Implementation --------------------------------------------------------------

Query layouts::insert_into(std::string table) {
    Query q(insert_layout);
    q |= clauses::identifier(&insert_into_slot, std::move(table));

    return q;
}
