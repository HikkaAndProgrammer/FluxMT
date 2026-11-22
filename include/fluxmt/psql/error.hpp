#pragma once

// std
#include <string_view>

// fluxmt
#include <fluxmt/core/error.hpp>
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    FLUXMT_EXPORT EDbError classify(std::string_view sqlstate) noexcept;
}
