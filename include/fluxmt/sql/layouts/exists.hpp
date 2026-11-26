#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/query/query.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query exists_in(std::string table);
}
