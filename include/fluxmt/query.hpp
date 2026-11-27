#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/conditional.hpp>
#include <fluxmt/query/context.hpp>
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/query/error.hpp>
#include <fluxmt/query/executor.hpp>
#include <fluxmt/query/layout.hpp>
#include <fluxmt/query/query.hpp>
#include <fluxmt/query/slot.hpp>
#include <fluxmt/query/slot_clause.hpp>
#include <fluxmt/query/statement.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::query {
    using impl::Clause;
    using impl::Context;
    using impl::Dialect;
    using impl::EArity;
    using impl::Layout;
    using impl::Query;
    using impl::SlotClause;
    
    using impl::clauses::when;
    using impl::make_clause;

    using impl::slot_info_t;
    using impl::slot_info_view;
    using impl::slot_ref;
    using impl::slot_t;
    using impl::statement_t;

    using impl::IsExecutor;
    using impl::IsStatementSource;
}


// Extension points: traits and polymorphic bases, nothing else

namespace flux::ext {
    using impl::Context;
    using impl::Dialect;
    using impl::IClause;
}
