#pragma once

// std
#include <string>
#include <string_view>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    // [a-z_][a-z0-9_]*
    FLUXMT_EXPORT bool is_plain_identifier(std::string_view name) noexcept;

    FLUXMT_EXPORT std::string quote_identifier(std::string_view name);
}
