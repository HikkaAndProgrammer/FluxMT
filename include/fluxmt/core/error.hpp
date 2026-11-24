#pragma once

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/core/type/error_code.hpp>

// export
#include <fluxmt/export.hpp>


// Macros ----------------------------------------------------------------------

#define FLUXMT_DB_ERRORS(X) \
    X(Other, "Database error") \
    X(ConnectionLost, "Connection lost") \
    X(PoolTimeout, "Timed out acquiring a pooled connection") \
    X(UniqueViolation, "Unique constraint violated") \
    X(ForeignKeyViolation, "Foreign key constraint violated") \
    X(CheckViolation, "Check constraint violated") \
    X(NotNullViolation, "Not-null constraint violated") \
    X(SerializationFailure, "Serialization failure") \
    X(Deadlock, "Deadlock detected") \
    X(Timeout, "Statement timed out") \
    X(Busy, "Database is busy or locked")


// Implementation --------------------------------------------------------------

ERS_MAKE_ERROR_CODES(flux::impl, EDbError, "db", FLUXMT_DB_ERRORS)

#undef FLUXMT_DB_ERRORS

namespace flux::impl {
    FLUXMT_EXPORT bool is_retryable(const std::error_code& code) noexcept;
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::EDbError;
    using impl::is_retryable;
}
