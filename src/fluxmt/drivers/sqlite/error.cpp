#include "fluxmt/drivers/sqlite/error.hpp"

// std
#include <exception>

// sqlite
#include <sqlite3.h>


// Implementation --------------------------------------------------------------

// error

flux::impl::sqlite::error::error(int extended_code, const std::string& message) :
    std::runtime_error(message),
    _code(extended_code) {
}

int flux::impl::sqlite::error::code() const noexcept { return _code; }


// Free functions

flux::impl::sqlite::error flux::impl::sqlite::last_error(sqlite3* db, int fallback) {
    if (!db)
        return { fallback, sqlite3_errstr(fallback) };
    return { sqlite3_extended_errcode(db), sqlite3_errmsg(db) };
}

flux::EDbError flux::impl::sqlite::classify(int extended_code) noexcept {
    switch (extended_code) {
        case SQLITE_CONSTRAINT_UNIQUE:
        case SQLITE_CONSTRAINT_PRIMARYKEY:
        case SQLITE_CONSTRAINT_ROWID:
            return EDbError::UniqueViolation;

        case SQLITE_CONSTRAINT_FOREIGNKEY:
            return EDbError::ForeignKeyViolation;

        case SQLITE_CONSTRAINT_CHECK:
            return EDbError::CheckViolation;

        case SQLITE_CONSTRAINT_NOTNULL:
            return EDbError::NotNullViolation;

        case SQLITE_BUSY_SNAPSHOT:
            return EDbError::SerializationFailure;
    }

    switch (extended_code & 0xFF) {
        case SQLITE_BUSY:
        case SQLITE_LOCKED:
            return EDbError::Busy;

        case SQLITE_INTERRUPT:
            return EDbError::Timeout;

        case SQLITE_CANTOPEN:
        case SQLITE_IOERR:
        case SQLITE_NOTADB:
            return EDbError::ConnectionLost;
    }

    return EDbError::Other;
}

ers::Diagnostic flux::impl::sqlite::diagnose(std::string_view label) try {
    throw;
} catch (const error& e) {
    auto kind = classify(e.code());

    if (kind == EDbError::ConnectionLost) {
        return ers::make_crit(kind, "'{}' failed [{}]: {}",
            label, e.code(), e.what());
    }

    return ers::make_error(kind, "'{}' failed [{}]: {}",
        label, e.code(), e.what());
} catch (const std::exception& e) {
    return ers::make_error(EDbError::Other, "'{}' failed: {}",
        label, e.what());
} catch (...) {
    return ers::make_error(EDbError::Other, "'{}' failed: unknown exception",
        label);
}

bool flux::impl::sqlite::retryable() noexcept try {
    throw;
} catch (const error& e) {
    return is_retryable(classify(e.code()));
} catch (...) {
    return false;
}
