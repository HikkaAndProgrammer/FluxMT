#include "fluxmt/sql/expr/expr.hpp"

// std
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// nodes
#include "nodes/call/fn.hpp"
#include "nodes/call/raw.hpp"
#include "nodes/call/row.hpp"
#include "nodes/logic/logical.hpp"
#include "nodes/logic/not.hpp"
#include "nodes/operator/binary.hpp"
#include "nodes/operator/cast.hpp"
#include "nodes/operator/ilike.hpp"
#include "nodes/operator/postfix.hpp"
#include "nodes/predicate/any.hpp"
#include "nodes/predicate/between.hpp"
#include "nodes/predicate/in.hpp"
#include "nodes/terminal/column.hpp"
#include "nodes/terminal/const.hpp"
#include "nodes/terminal/param.hpp"
#include "nodes/terminal/value.hpp"


// Expr ------------------------------------------------------------------------

// Member functions

flux::impl::Expr::Expr(ExprNodePtr node) :
    _node(std::move(node)) {
}


// Observers

flux::impl::EPrecedence flux::impl::Expr::precedence() const noexcept {
    return _node->precedence();
}

const flux::impl::IExprNode& flux::impl::Expr::node() const noexcept { return *_node; }


// Generators

flux::impl::Expr flux::impl::Expr::is_null() const {
    return make_postfix(*this, " IS NULL", EPrecedence::Is);
}
flux::impl::Expr flux::impl::Expr::is_not_null() const {
    return make_postfix(*this, " IS NOT NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::Expr::is_distinct_from(Expr other) const {
    return make_binary("IS DISTINCT FROM", *this, std::move(other), EPrecedence::Is);
}


flux::impl::Expr flux::impl::Expr::like(Expr pattern) const {
    return make_binary("LIKE", *this, std::move(pattern), EPrecedence::Like);
}

flux::impl::Expr flux::impl::Expr::ilike(Expr pattern) const {
    return make_ilike(*this, std::move(pattern));
}


flux::impl::Expr flux::impl::Expr::between(Expr low, Expr high) const {
    return make_between(*this, std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::Expr::contains(Expr other) const {
    return make_binary("@>", *this, std::move(other), EPrecedence::Other);
}

flux::impl::Expr flux::impl::Expr::in(std::initializer_list<Expr> values) const {
    return make_in(*this, std::vector(values));
}


flux::impl::Expr flux::impl::Expr::any(Expr array) const {
    return make_any(*this, std::move(array));
}

flux::impl::Expr flux::impl::Expr::cast(std::string_view type) const {
    return make_cast(*this, type);
}


// Executors

ers::Status flux::impl::Expr::render(Context& ctx, EPrecedence parent) const {
    if (precedence() >= parent)
        return _node->render(ctx);

    ctx.query += '(';
    ERS_TRY(_node->render(ctx));

    ctx.query += ')';

    return ers::ok;
}


// Node factories --------------------------------------------------------------

flux::impl::Expr flux::impl::make_value(Value value) {
    return make_expr<nodes::ValueNode>(std::move(value));
}

flux::impl::Expr flux::impl::make_param(Value value) {
    return make_expr<nodes::ParamNode>(std::move(value));
}

flux::impl::Expr flux::impl::make_const(sql_text text) {
    return make_expr<nodes::ConstNode>(text.text);
}

flux::impl::Expr flux::impl::make_postfix(Expr operand, sql_text suffix, EPrecedence precedence) {
    return make_expr<nodes::PostfixNode>(std::move(operand), suffix.text, precedence);
}

flux::impl::Expr flux::impl::make_between(Expr operand, Expr low, Expr high) {
    return make_expr<nodes::BetweenNode>(std::move(operand), std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::make_in(Expr operand, std::vector<Expr> items) {
    return make_expr<nodes::InNode>(std::move(operand), std::move(items));
}

flux::impl::Expr flux::impl::make_any(Expr operand, Expr array) {
    return make_expr<nodes::AnyNode>(std::move(operand), std::move(array));
}

flux::impl::Expr flux::impl::make_cast(Expr operand, std::string_view type) {
    return make_expr<nodes::CastNode>(std::move(operand), std::string(type));
}

flux::impl::Expr flux::impl::make_ilike(Expr lhs, Expr rhs) {
    return make_expr<nodes::IlikeNode>(std::move(lhs), std::move(rhs));
}

flux::impl::Expr flux::impl::make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative) {
    return make_expr<nodes::BinaryNode>(std::string(op), std::move(lhs), std::move(rhs), precedence, left_associative);
}

flux::impl::Expr flux::impl::make_fn(std::string_view name, std::vector<Expr> args) {
    return make_expr<nodes::FnNode>(name, std::move(args));
}

flux::impl::Expr flux::impl::make_raw(sql_text pattern, std::vector<Expr> args) {
    return make_expr<nodes::RawNode>(pattern.text, std::move(args));
}

flux::impl::Expr flux::impl::make_row(std::vector<Expr> items) {
    return make_expr<nodes::RowNode>(std::move(items));
}

flux::impl::Expr flux::impl::make_logical(bool conjunction, Expr lhs, Expr rhs) {
    std::vector<Expr> items;

    for (auto* side : { &lhs, &rhs }) {
        const auto* logical = dynamic_cast<const nodes::LogicalNode*>(&side->node());

        if (logical && logical->conjunction() == conjunction)
            items.append_range(logical->items());
        else
            items.push_back(std::move(*side));
    }

    return make_expr<nodes::LogicalNode>(conjunction, std::move(items));
}

flux::impl::Expr flux::impl::make_not(Expr what) {
    return make_expr<nodes::NotNode>(std::move(what));
}


// Shortcuts -------------------------------------------------------------------

flux::impl::Expr flux::impl::clauses::col(std::string name) {
    return make_expr<nodes::ColumnNode>(std::move(name));
}

flux::impl::Expr flux::impl::clauses::col(std::string_view table, std::string_view name) {
    return col(std::string(table) + '.' + std::string(name));
}

flux::impl::Expr flux::impl::clauses::null() {
    return make_const("NULL");
}

flux::impl::Expr flux::impl::clauses::sql_bool(bool value) {
    return value ? make_const("TRUE") : make_const("FALSE");
}
