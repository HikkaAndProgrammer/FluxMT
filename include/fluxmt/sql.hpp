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
#include <fluxmt/sql/syntax.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::Aliased;
    using impl::ColumnExpr;
    using impl::Context;
    using impl::Dialect;
    using impl::EOrder;
    using impl::ESyntax;
    using impl::Expr;
    using impl::ExprMethods;
    using impl::IsExprSource;
    using impl::Query;
    using impl::SqlBackend;
    using impl::Statement;
    using impl::TypedExpr;
    using impl::WithCte;
    using impl::WithUnion;
    using impl::function_spelling_t;
    using impl::layout_entry_t;
    using impl::needs_t;
    using impl::syntax_t;

    using impl::missing;
    using impl::needs_of;
    using impl::no_needs;
    using impl::supports;

    using impl::is_plain_identifier;
    using impl::quote_identifier;
    using impl::is_valid_name;
    using impl::is_type_name;
}


namespace flux {
    namespace slots = impl::slots;
    namespace kinds = impl::kinds;
    namespace syntax = impl::syntax;
}


// Extension points: your own nodes, operators, functions and clauses.
namespace flux::ext {
    using impl::Clause;
    using impl::Context;
    using impl::EArity;
    using impl::EPrecedence;
    using impl::IClause;
    using impl::IExprNode;
    using impl::KindRef;
    using impl::Layout;
    using impl::LayoutView;
    using impl::Slot;
    using impl::SlotBinding;
    using impl::SlotRef;
    using impl::StatementKind;
    using impl::binary_op_t;
    using impl::find_layout;
    using impl::fn_t;
    using impl::make_clause;
    using impl::make_expr;
    using impl::need_function;
    using impl::need_syntax;
    using impl::retype;
    using impl::sql_text;
}


// The query language: 'using namespace flux::orm;'.
namespace flux::orm {
    using namespace impl::clauses;
    using namespace impl::layouts;

    using impl::EOrder;
}
