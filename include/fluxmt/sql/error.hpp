#pragma once

// ers
#include <erslib/core/type/error_code.hpp>

// fluxmt
#include <fluxmt/query/error.hpp>


// Macros ----------------------------------------------------------------------

#define FLUXMT_SQL_ERRORS(X) \
    X(InvalidIdentifier, "Invalid identifier") \
    X(InvalidName, "Invalid function, aggregate or type name") \
    X(MissingOrdering, "Aggregate needs an ordering") \
    X(RawPlaceholderMismatch, "Raw SQL placeholders do not match the arguments") \
    X(NegativeBound, "Negative bound") \
    X(EmptyColumns, "Slot needs at least one column") \
    X(EmptyValues, "Slot needs at least one value") \
    X(PlaceholderMismatch, "Slot markers do not match the values") \
    X(InvalidSetItem, "Only assignments are allowed here")


// Implementation --------------------------------------------------------------

ERS_MAKE_ERROR_CODES(flux::impl, ESqlError, "sql", FLUXMT_SQL_ERRORS)

#undef FLUXMT_SQL_ERRORS


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::ESqlError;
}
