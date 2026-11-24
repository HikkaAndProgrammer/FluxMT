#include "fluxmt/core/error.hpp"


bool flux::impl::is_retryable(const std::error_code& code) noexcept {
    return code == EDbError::SerializationFailure
        || code == EDbError::Deadlock
        || code == EDbError::Busy;
}
