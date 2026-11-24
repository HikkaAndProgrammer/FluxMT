#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/sql/backend.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/identifier.hpp>
#include <fluxmt/sql/statement.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::Context;
    using impl::Dialect;
    using impl::SqlBackend;
    using impl::Statement;

    using impl::is_plain_identifier;
    using impl::quote_identifier;
}
