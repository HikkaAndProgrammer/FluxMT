#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/sql/identifier.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sql {
    using impl::is_plain_identifier;
    using impl::quote_identifier;
}
