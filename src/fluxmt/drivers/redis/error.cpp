#include "fluxmt/drivers/redis/error.hpp"

// std
#include <exception>

// hiredis
#include <hiredis/hiredis.h>


// Implementation --------------------------------------------------------------

// error

flux::impl::redis::error::error(EDbError kind, const std::string& message) :
    std::runtime_error(message),
    _kind(kind) {
}

flux::EDbError flux::impl::redis::error::kind() const noexcept {
    return _kind;
}


// Free functions

flux::EDbError flux::impl::redis::classify_reply(std::string_view text) noexcept {
    const auto prefix = text.substr(0, text.find(' '));

    if (prefix == "BUSY" || prefix == "LOADING" || prefix == "TRYAGAIN")
        return EDbError::Busy;

    return EDbError::Other;
}

flux::EDbError flux::impl::redis::classify_transport(int hiredis_error) noexcept {
    switch (hiredis_error) {
        case REDIS_ERR_TIMEOUT:
            return EDbError::Timeout;

        case REDIS_ERR_IO:
        case REDIS_ERR_EOF:
        case REDIS_ERR_PROTOCOL:
        case REDIS_ERR_OTHER:
            return EDbError::ConnectionLost;

        default:
            return EDbError::Other;
    }
}

ers::Diagnostic flux::impl::redis::diagnose(std::string_view label) try {
    throw;
} catch (const error& e) {
    if (e.kind() == EDbError::ConnectionLost) {
        return ers::make_crit(e.kind(), "'{}' failed: {}",
            label, e.what());
    }

    return ers::make_error(e.kind(), "'{}' failed: {}",
        label, e.what());
} catch (const std::exception& e) {
    return ers::make_error(EDbError::Other, "'{}' failed: {}",
        label, e.what());
} catch (...) {
    return ers::make_error(EDbError::Other, "'{}' failed: unknown exception",
        label);
}

bool flux::impl::redis::retryable() noexcept try {
    throw;
} catch (const error& e) {
    return is_retryable(e.kind());
} catch (...) {
    return false;
}
