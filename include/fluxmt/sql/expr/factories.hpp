#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/expr/fwd.hpp>
#include <fluxmt/sql/expr/literal.hpp>
#include <fluxmt/sql/expr/node.hpp>

// export
#include <fluxmt/export.hpp>


// The templates that need a complete Expr (make_expr, val, param, row) are in expr.hpp.

namespace flux::impl {
    FLUXMT_EXPORT Expr make_value(Value value);
    FLUXMT_EXPORT Expr make_param(Value value);
    FLUXMT_EXPORT Expr make_const(sql_text text);
    FLUXMT_EXPORT Expr make_postfix(Expr operand, sql_text suffix, EPrecedence precedence);
    FLUXMT_EXPORT Expr make_between(Expr operand, Expr low, Expr high);
    FLUXMT_EXPORT Expr make_in(Expr operand, std::vector<Expr> items);
    FLUXMT_EXPORT Expr make_in(Expr operand, Query subquery);
    FLUXMT_EXPORT Expr make_any(Expr operand, Expr array);
    FLUXMT_EXPORT Expr make_cast(Expr operand, std::string_view type);
    FLUXMT_EXPORT Expr make_ilike(Expr lhs, Expr rhs);
    FLUXMT_EXPORT Expr make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative = false);
    FLUXMT_EXPORT Expr make_fn(std::string_view name, std::vector<Expr> args);
    FLUXMT_EXPORT Expr make_raw(sql_text pattern, std::vector<Expr> args);
    FLUXMT_EXPORT Expr make_row(std::vector<Expr> items);
    FLUXMT_EXPORT Expr make_logical(bool conjunction, Expr lhs, Expr rhs);
    FLUXMT_EXPORT Expr make_not(Expr what);


    namespace clauses {
        FLUXMT_EXPORT Expr col(std::string name);
        FLUXMT_EXPORT Expr col(std::string_view table, std::string_view name);
        FLUXMT_EXPORT Expr null();
        FLUXMT_EXPORT Expr sql_bool(bool value);

        FLUXMT_EXPORT Expr exists(Query subquery);
        FLUXMT_EXPORT Expr subquery(Query query);
    }
}
