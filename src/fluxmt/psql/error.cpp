#include "fluxmt/psql/error.hpp"


flux::impl::EDbError flux::impl::psql::classify(std::string_view sqlstate) noexcept {
    if (sqlstate == "23505") return EDbError::UniqueViolation;
    if (sqlstate == "23503") return EDbError::ForeignKeyViolation;
    if (sqlstate == "23514") return EDbError::CheckViolation;
    if (sqlstate == "23502") return EDbError::NotNullViolation;
    if (sqlstate == "40001") return EDbError::SerializationFailure;
    if (sqlstate == "40P01") return EDbError::Deadlock;
    if (sqlstate == "57014") return EDbError::Timeout;
    if (sqlstate.starts_with("08")) return EDbError::ConnectionLost;

    return EDbError::Other;
}
