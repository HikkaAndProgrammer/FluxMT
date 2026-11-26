#include "fluxmt/sql/syntax.hpp"

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>

// nodes
#include "expr/nodes/common.hpp"
#include "expr/nodes/terminal/bound.hpp"


using namespace flux::impl;


// Implementation --------------------------------------------------------------

// cast

ers::Status syntax::cast_postfix(Context& ctx, const Expr& operand, std::string_view type) {
    ERS_TRY(operand.render(ctx, EPrecedence::Cast));

    ctx.query += "::";
    ctx.query += type;

    return ers::ok;
}

ers::Status syntax::cast_function(Context& ctx, const Expr& operand, std::string_view type) {
    ctx.query += "CAST(";
    ERS_TRY(operand.render(ctx));

    ctx.query += " AS ";
    ctx.query += type;
    ctx.query += ')';

    return ers::ok;
}


// ilike

ers::Status syntax::ilike_operator(Context& ctx, const Expr& lhs, const Expr& rhs) {
    constexpr auto operand = nodes::next(EPrecedence::Like);

    ERS_TRY(lhs.render(ctx, operand));

    ctx.query += " ILIKE ";

    return rhs.render(ctx, operand);
}

ers::Status syntax::ilike_via_lower(Context& ctx, const Expr& lhs, const Expr& rhs) {
    ctx.query += "lower(";
    ERS_TRY(lhs.render(ctx));

    ctx.query += ") LIKE lower(";
    ERS_TRY(rhs.render(ctx));

    ctx.query += ')';

    return ers::ok;
}


// any

ers::Status syntax::any_native(Context& ctx, const Expr& operand, const Expr& array) {
    ERS_TRY(operand.render(ctx, EPrecedence::Like));

    ctx.query += " = ANY(";
    ERS_TRY(array.render(ctx));

    ctx.query += ')';

    return ers::ok;
}

ers::Status syntax::any_as_in_list(Context& ctx, const Expr& operand, const Expr& array) {
    const auto* bound = dynamic_cast<const nodes::BoundNode*>(&array.node());
    const auto* items = bound ? bound->value().get_if<value_array_t>() : nullptr;

    if (!items) {
        return ers::make_error(ESqlError::Unsupported, "ANY needs an array value, '{}' has none",
            ctx.dialect().name());
    }

    if (items->empty()) {
        ctx.query += "FALSE";

        return ers::ok;
    }

    ERS_TRY(operand.render(ctx, EPrecedence::Other));

    ctx.query += " IN (";

    bool first = true;
    for (const auto& it : *items) {
        if (!first)
            ctx.query += ", ";
        first = false;

        ctx.query += ctx.bind(it);
    }

    ctx.query += ')';

    return ers::ok;
}


// upsert

ers::Status syntax::excluded_table(Context& ctx, std::string_view column) {
    ctx.query += "excluded.";

    return ctx.append_identifier(column);
}
