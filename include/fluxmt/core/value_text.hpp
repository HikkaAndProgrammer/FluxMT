#pragma once

// std
#include <string>

// ers
#include <erslib/core/type/time.hpp>
#include <erslib/core/type/uuid.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    // Text forms of the values SQLite and Redis keep as plain strings.
    FLUXMT_EXPORT std::string to_text(const ers::date_t& value);
    FLUXMT_EXPORT std::string to_text(const ers::timestamp_t& value);
    FLUXMT_EXPORT std::string to_text(const ers::uuid& value);
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::to_text;
}
