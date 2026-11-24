#pragma once

// fluxmt
#include <fluxmt/core/value.hpp>

// export
#include <fluxmt/export.hpp>

struct sqlite3_stmt;


namespace flux::impl::sqlite {
    FLUXMT_EXPORT void bind(::sqlite3_stmt* stmt, int index, const Value& value);
}
