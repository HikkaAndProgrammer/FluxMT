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
#include <fluxmt/sql/query.hpp>
#include <fluxmt/sql/statement.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::Aliased;
    using impl::ColumnExpr;
    using impl::Context;
    using impl::Dialect;
    using impl::ECastStyle;
    using impl::EOrder;
    using impl::Expr;
    using impl::ExprMethods;
    using impl::IsExprSource;
    using impl::Query;
    using impl::SqlBackend;
    using impl::Statement;
    using impl::WithCte;
    using impl::WithUnion;
    using impl::dialect_caps_t;

    using impl::is_plain_identifier;
    using impl::quote_identifier;
    using impl::is_valid_name;
    using impl::is_type_name;
}


namespace flux {
    namespace slots = impl::slots;
    namespace bindings = impl::bindings;
}


// Extension points: your own nodes, operators, functions and clauses.
namespace flux::ext {
    using impl::Clause;
    using impl::Context;
    using impl::EArity;
    using impl::EPrecedence;
    using impl::IClause;
    using impl::IExprNode;
    using impl::Layout;
    using impl::Slot;
    using impl::SlotBinding;
    using impl::SlotRef;
    using impl::binary_op_t;
    using impl::fn_t;
    using impl::make_clause;
    using impl::make_expr;
    using impl::sql_text;
}


// The query language: 'using namespace flux::orm;'.
namespace flux::orm {
    using namespace impl::clauses;
    using namespace impl::layouts;

    using impl::EOrder;
}
