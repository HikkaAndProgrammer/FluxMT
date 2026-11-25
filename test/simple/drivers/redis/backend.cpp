// catch2
#include <catch2/catch_test_macros.hpp>

// fluxmt
#include <fluxmt/drivers/redis.hpp>

// test
#include "support/test_case.hpp"


using backend = flux::redis::backend;

static_assert(flux::Backend<backend>);
static_assert(flux::Connectable<backend>);
static_assert(flux::NativeTransaction<flux::redis::transaction, backend>);
static_assert(flux::Supports<backend, flux::EFeature::Transactions>);
static_assert(!flux::Supports<backend, flux::EFeature::Sql>);
static_assert(!flux::Supports<backend, flux::EFeature::Returning>);
static_assert(!flux::Supports<backend, flux::EFeature::Upsert>);
static_assert(!flux::Supports<backend, flux::EFeature::NativeArrays>);


FLUXMT_TEST_CASE("redis backend traits", "[redis]") {
    using traits = flux::impl::backend_traits<backend>;

    REQUIRE(traits::name == "redis");
    REQUIRE(traits::features.contains(flux::EFeature::Transactions));
    REQUIRE(!traits::features.contains(flux::EFeature::Sql));
}
