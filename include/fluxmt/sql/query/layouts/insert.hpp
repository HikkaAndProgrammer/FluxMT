#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/sql/query/query.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query insert_into(std::string table);
}
