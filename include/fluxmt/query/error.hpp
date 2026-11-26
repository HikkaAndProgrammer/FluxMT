#pragma once

// ers
#include <erslib/core/type/error_code.hpp>


// Macros ----------------------------------------------------------------------

#define FLUXMT_QUERY_ERRORS(X) \
    X(Unsupported, "The dialect cannot express it") \
    X(NullSlot, "Null slot") \
    X(UnknownAnchor, "Anchor slot is not in the layout") \
    X(DuplicateSlot, "Slot is already in the layout") \
    X(ForeignSlot, "Slot is not part of this statement") \
    X(SlotConflict, "Slots cannot be used together")


// Implementation --------------------------------------------------------------

ERS_MAKE_ERROR_CODES(flux::impl, EQueryError, "query", FLUXMT_QUERY_ERRORS)

#undef FLUXMT_QUERY_ERRORS


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::EQueryError;
}
