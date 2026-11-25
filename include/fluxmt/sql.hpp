#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/sql/backend.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/aggregate.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/functions.hpp>
#include <fluxmt/sql/expr/named.hpp>
#include <fluxmt/sql/expr/operators.hpp>
#include <fluxmt/sql/identifier.hpp>
#include <fluxmt/sql/statement.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::Aliased;
    using impl::ColumnExpr;
    using impl::Context;
    using impl::Dialect;
    using impl::ECastStyle;
    using impl::Expr;
    using impl::ExprMethods;
    using impl::IsExprSource;
    using impl::SqlBackend;
    using impl::Statement;
    using impl::dialect_caps_t;

    using impl::is_plain_identifier;
    using impl::quote_identifier;
    using impl::is_valid_name;
    using impl::is_type_name;
}


// Extension points: your own nodes, operators and functions.
namespace flux::ext {
    using impl::Context;
    using impl::EPrecedence;
    using impl::IExprNode;
    using impl::binary_op_t;
    using impl::fn_t;
    using impl::make_expr;
    using impl::sql_text;
}


// The expression language: 'using namespace flux::orm;'.
namespace flux::orm {
    using impl::clauses::agg;
    using impl::clauses::agg_distinct;
    using impl::clauses::array_agg_of;
    using impl::clauses::avg_of;
    using impl::clauses::bool_and_of;
    using impl::clauses::bool_or_of;
    using impl::clauses::coalesce;
    using impl::clauses::col;
    using impl::clauses::concat;
    using impl::clauses::corr_of;
    using impl::clauses::count_all;
    using impl::clauses::count_distinct;
    using impl::clauses::count_of;
    using impl::clauses::greatest;
    using impl::clauses::json_rows;
    using impl::clauses::least;
    using impl::clauses::max_of;
    using impl::clauses::min_of;
    using impl::clauses::mode_of;
    using impl::clauses::now;
    using impl::clauses::null;
    using impl::clauses::nullif;
    using impl::clauses::param;
    using impl::clauses::percentile_cont;
    using impl::clauses::percentile_disc;
    using impl::clauses::row;
    using impl::clauses::sql;
    using impl::clauses::sql_bool;
    using impl::clauses::stddev_of;
    using impl::clauses::string_agg;
    using impl::clauses::sum_of;
    using impl::clauses::val;
    using impl::clauses::variance_of;
}
