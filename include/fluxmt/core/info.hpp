#pragma once

// std
#include <string_view>

// fluxmt
#include <fluxmt/export.hpp>
#include <fluxmt/version.hpp>


namespace flux {
    FLUXMT_EXPORT std::string_view library_version() noexcept;
}
