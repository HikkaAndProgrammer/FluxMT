#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/conditional.hpp>
#include <fluxmt/query/context.hpp>
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/query/error.hpp>
#include <fluxmt/query/kind.hpp>
#include <fluxmt/query/layout.hpp>
#include <fluxmt/query/query.hpp>
#include <fluxmt/query/slot.hpp>
#include <fluxmt/query/statement.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::query {
    using impl::Context;
    using impl::Dialect;
    using impl::kind_ref;
    using impl::Layout;
    using impl::slot_info_view;
    using impl::Query;
    using impl::statement_kind_t;
    using impl::layout_entry_t;
    using impl::find_layout;
    using impl::statement_t;
}


// Extension points

namespace flux::ext {
    using impl::Clause;
    using impl::Context;
    using impl::EArity;
    using impl::IClause;
    using impl::kind_ref;
    using impl::Layout;
    using impl::slot_info_view;
    using impl::slot_t;
    using impl::slot_info_t;
    using impl::slot_ref;
    using impl::statement_kind_t;
    using impl::find_layout;
    using impl::make_clause;
}
