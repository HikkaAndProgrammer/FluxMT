#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/core/value.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::redis {
    FLUXMT_EXPORT std::string to_arg(const Value& value);
}
