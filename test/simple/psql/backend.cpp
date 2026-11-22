// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <concepts>

// pqxx
#include <pqxx/connection>

// fluxmt
#include <fluxmt/psql.hpp>

// test
#include "support/test_case.hpp"


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("psql backend", "[psql]") {
    using B = flux::psql::backend;

    STATIC_REQUIRE(flux::Backend<B>);
    STATIC_REQUIRE(std::same_as<flux::connection_t<B>, pqxx::connection>);
    STATIC_REQUIRE(flux::impl::backend_traits<B>::name == "psql");

    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::Sql>);
    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::Transactions>);
    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::Returning>);
    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::Upsert>);
    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::DistinctOn>);
    STATIC_REQUIRE(flux::Supports<B, flux::EFeature::NativeArrays>);
}
