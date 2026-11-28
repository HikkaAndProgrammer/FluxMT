#include "fluxmt/sql/clauses/assign.hpp"

// std
#include <cstddef>
#include <vector>

// fluxmt
#include <fluxmt/sql/error.hpp>

// clauses
#include "assign.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_assign_clause(std::string column, Expr value) {
    return make_clause<AssignClause>(std::move(column), std::move(value));
}

ers::Status flux::impl::render_set_as_values(Context& base, std::span<const Clause* const> group) {
    auto& ctx = sql_of(base);

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

    ctx.text += " (";

    for (size_t i = 0; i < assigns.size(); ++i) {
        if (i != 0)
            ctx.text += ", ";

        ERS_TRY(assigns[i]->render_column(ctx));
    }

    ctx.text += ")\nVALUES (";

    for (size_t i = 0; i < assigns.size(); ++i) {
        if (i != 0)
            ctx.text += ", ";

        ERS_TRY(assigns[i]->render_value(ctx));
    }

    ctx.text += ")";

    return ers::ok;
}


// Shortcuts

flux::impl::Clause flux::impl::make_assign_null_clause(std::string column) {
    return make_assign_clause(std::move(column), clauses::null());
}
