#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/query.hpp>
#include <fluxmt/sql/backend.hpp>
#include <fluxmt/sql/builder.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/aggregate.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/functions.hpp>
#include <fluxmt/sql/expr/named.hpp>
#include <fluxmt/sql/expr/operators.hpp>
#include <fluxmt/sql/identifier.hpp>
#include <fluxmt/sql/syntax.hpp>

#include <fluxmt/sql/slots/column.hpp>
#include <fluxmt/sql/slots/conflict.hpp>
#include <fluxmt/sql/slots/distinct.hpp>
#include <fluxmt/sql/slots/from.hpp>
#include <fluxmt/sql/slots/group.hpp>
#include <fluxmt/sql/slots/having.hpp>
#include <fluxmt/sql/slots/head.hpp>
#include <fluxmt/sql/slots/join.hpp>
#include <fluxmt/sql/slots/limit.hpp>
#include <fluxmt/sql/slots/offset.hpp>
#include <fluxmt/sql/slots/order.hpp>
#include <fluxmt/sql/slots/returning.hpp>
#include <fluxmt/sql/slots/set.hpp>
#include <fluxmt/sql/slots/values.hpp>
#include <fluxmt/sql/slots/where.hpp>
#include <fluxmt/sql/slots/with.hpp>

#include <fluxmt/sql/clauses/assign.hpp>
#include <fluxmt/sql/clauses/bound.hpp>
#include <fluxmt/sql/clauses/column.hpp>
#include <fluxmt/sql/clauses/conflict.hpp>
#include <fluxmt/sql/clauses/distinct.hpp>
#include <fluxmt/sql/clauses/group.hpp>
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/clauses/join.hpp>
#include <fluxmt/sql/clauses/order.hpp>
#include <fluxmt/sql/clauses/raw.hpp>
#include <fluxmt/sql/clauses/returning.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/clauses/values.hpp>
#include <fluxmt/sql/clauses/where.hpp>
#include <fluxmt/sql/clauses/with.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::aliased_t;
    using impl::column_expr_t;
    using impl::Expr;
    using impl::ExprMethods;
    using impl::IsExprSource;
    using impl::TypedExpr;

    using impl::IsSqlBackend;
    using impl::QueryBuilder;
    using impl::SqlClause;
    using impl::SqlClauses;
    using impl::clause_needs;

    using impl::ESyntax;
    using impl::function_rule_t;
    using impl::function_rules_t;
    using impl::function_spelling_t;
    using impl::syntax_t;

    using impl::needs_t;
    using impl::missing;
    using impl::needs_of;
    using impl::no_needs;
    using impl::spelling_of;
    using impl::supports;
    using impl::with_functions;

    using impl::is_plain_identifier;
    using impl::quote_identifier;
    using impl::is_valid_name;
    using impl::is_type_name;

    using impl::EPrecedence;
    using impl::binary_op_fn;
    using impl::named_fn;
    using impl::make_expr;
    using impl::need_function;
    using impl::need_syntax;
    using impl::retype;
    using impl::sql_of;
    using impl::sql_text;
}


namespace flux {
    namespace slots = impl::slots;
    namespace syntax = impl::syntax;
}


// Extension points: traits and polymorphic bases, nothing else

namespace flux::ext {
    using impl::IExprNode;
    using impl::SqlContext;
    using impl::SqlDialect;
}


// The clause vocabulary; statements come from the orm of a backend.

namespace flux::orm {
    using namespace impl::clauses;

    using impl::EOrder;
}
