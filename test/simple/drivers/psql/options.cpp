// catch2
#include <catch2/catch_test_macros.hpp>

// fluxmt
#include <fluxmt/drivers/psql.hpp>

// test
#include "support/test_case.hpp"


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("connection string", "[psql]") {
    const flux::psql::options_t opts {
        .dbname   = "shop",
        .username = "shop",
        .password = "it's",
    };

    REQUIRE(opts.connection_string() == R"(host='localhost' port=5432 dbname='shop' user='shop' password='it\'s')");
}

FLUXMT_TEST_CASE("empty fields are left out", "[psql]") {
    REQUIRE(flux::psql::options_t {}.connection_string() == "host='localhost' port=5432");
}
