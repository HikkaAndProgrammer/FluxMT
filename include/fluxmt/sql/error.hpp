#pragma once

// ers
#include <erslib/core/type/error_code.hpp>


// Macros ----------------------------------------------------------------------

#define FLUXMT_SQL_ERRORS(X) \
    X(InvalidIdentifier, "Invalid identifier") \
    X(InvalidName, "Invalid function, aggregate or type name") \
    X(MissingOrdering, "Aggregate needs an ordering") \
    X(RawPlaceholderMismatch, "Raw SQL placeholders do not match the arguments") \
    X(Unsupported, "The dialect cannot express it") \
    X(NullSlot, "Null slot") \
    X(UnknownAnchor, "Anchor slot is not in the layout") \
    X(DuplicateSlot, "Slot is already in the layout") \
    X(ForeignSlot, "Slot is not part of this statement") \
    X(SlotConflict, "Slots cannot be used together") \
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
