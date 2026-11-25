// catch2
#include <catch2/catch_test_macros.hpp>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>

// test
#include "support/test_case.hpp"


using backend = flux::sqlite::backend;

static_assert(flux::Backend<backend>);
static_assert(flux::Connectable<backend>);
static_assert(flux::Supports<backend, flux::EFeature::Returning>);
static_assert(flux::Supports<backend, flux::EFeature::Upsert>);
static_assert(!flux::Supports<backend, flux::EFeature::DistinctOn>);
static_assert(!flux::Supports<backend, flux::EFeature::NativeArrays>);


FLUXMT_TEST_CASE("sqlite backend traits", "[sqlite]") {
    using traits = flux::impl::backend_traits<backend>;

    REQUIRE(traits::name == "sqlite");
    REQUIRE(traits::features.contains(flux::EFeature::Sql));
    REQUIRE(traits::features.contains(flux::EFeature::Transactions));
    REQUIRE(!traits::features.contains(flux::EFeature::DistinctOn));
}
