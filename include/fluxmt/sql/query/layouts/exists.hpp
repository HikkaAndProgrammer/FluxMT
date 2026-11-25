#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/sql/query/query.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::layouts {
    // SELECT EXISTS (SELECT 1 FROM <table> [WHERE ...]) AS "exists"
    FLUXMT_EXPORT Query exists_in(std::string table);
}
