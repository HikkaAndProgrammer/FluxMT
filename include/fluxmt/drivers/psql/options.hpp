#pragma once

// std
#include <string>

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    struct FLUXMT_EXPORT options_t {
        std::string host = "localhost";
        ers::u16 port = 5432;
        std::string dbname;
        std::string username;
        std::string password;


        // libpq key/value connection string built from the fields above.
        [[nodiscard]]
        std::string connection_string() const;
    };
}
