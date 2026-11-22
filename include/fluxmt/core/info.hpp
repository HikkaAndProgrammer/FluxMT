#pragma once

// std
#include <string_view>

// fluxmt
#include <fluxmt/export.hpp>
#include <fluxmt/version.hpp>


namespace flux {
    // The version the library was built with, against flux::version from the headers.
    FLUXMT_EXPORT std::string_view library_version() noexcept;
}
