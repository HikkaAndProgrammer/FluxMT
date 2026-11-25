#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/sql/query/query.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query select_from(std::string table, std::string alias = "");

    // SELECT with no FROM
    FLUXMT_EXPORT Query select();
}
