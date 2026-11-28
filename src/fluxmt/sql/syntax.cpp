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

ers::Status syntax::cast_postfix(SqlContext& ctx, const Expr& operand, std::string_view type) {
    ERS_TRY(operand.render(ctx, EPrecedence::Cast));

    ctx.text += "::";
    ctx.text += type;

    return ers::ok;
}

ers::Status syntax::cast_function(SqlContext& ctx, const Expr& operand, std::string_view type) {
    ctx.text += "CAST(";
    ERS_TRY(operand.render(ctx));

    ctx.text += " AS ";
    ctx.text += type;
    ctx.text += ')';

    return ers::ok;
}


// ilike

ers::Status syntax::ilike_operator(SqlContext& ctx, const Expr& lhs, const Expr& rhs) {
    constexpr auto operand = nodes::next(EPrecedence::Like);

    ERS_TRY(lhs.render(ctx, operand));

    ctx.text += " ILIKE ";

    return rhs.render(ctx, operand);
}

ers::Status syntax::ilike_via_lower(SqlContext& ctx, const Expr& lhs, const Expr& rhs) {
    ctx.text += "lower(";
    ERS_TRY(lhs.render(ctx));

    ctx.text += ") LIKE lower(";
    ERS_TRY(rhs.render(ctx));

    ctx.text += ')';

    return ers::ok;
}


// any

ers::Status syntax::any_native(SqlContext& ctx, const Expr& operand, const Expr& array) {
    ERS_TRY(operand.render(ctx, EPrecedence::Like));

    ctx.text += " = ANY(";
    ERS_TRY(array.render(ctx));

    ctx.text += ')';

    return ers::ok;
}

ers::Status syntax::any_as_in_list(SqlContext& ctx, const Expr& operand, const Expr& array) {
    const auto* bound = dynamic_cast<const nodes::BoundNode*>(&array.node());
    const auto* items = bound ? bound->value().get_if<value_array_t>() : nullptr;

    if (!items) {
        return ers::make_error(EQueryError::Unsupported, "ANY needs an array value, '{}' has none",
            ctx.dialect().name());
    }

    if (items->empty()) {
        ctx.text += "FALSE";

        return ers::ok;
    }

    ERS_TRY(operand.render(ctx, EPrecedence::Other));

    ctx.text += " IN (";

    bool first = true;
    for (const auto& it : *items) {
        if (!first)
            ctx.text += ", ";
        first = false;

        ctx.text += ctx.bind(it);
    }

    ctx.text += ')';

    return ers::ok;
}


// upsert

ers::Status syntax::excluded_table(SqlContext& ctx, std::string_view column) {
    ctx.text += "excluded.";

    return ctx.append_identifier(column);
}
