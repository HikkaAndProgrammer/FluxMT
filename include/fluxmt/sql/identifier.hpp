#pragma once

// std
#include <string>
#include <string_view>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT bool is_plain_identifier(std::string_view name) noexcept;
    FLUXMT_EXPORT bool is_valid_name(std::string_view name) noexcept;
    FLUXMT_EXPORT bool is_type_name(std::string_view name) noexcept;

    FLUXMT_EXPORT std::string quote_identifier(std::string_view name);
}
