// catch2
#include <catch2/catch_test_macros.hpp>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>
#include <fluxmt/sql.hpp>

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

FLUXMT_TEST_CASE("sqlite backend is a sql backend at compile time", "[sqlite]") {
    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::SqlBackend<backend>);

    STATIC_REQUIRE(impl::HasKind<backend, &flux::kinds::select>);

    STATIC_REQUIRE(impl::HasSlot<backend, &flux::kinds::insert, &flux::slots::returning>);
    STATIC_REQUIRE(!impl::HasSlot<backend, &flux::kinds::insert, &flux::slots::conflict_constraint>);
    STATIC_REQUIRE(!impl::HasSlot<backend, &flux::kinds::select, &flux::slots::returning>);

    STATIC_REQUIRE(impl::SupportsNeeds<backend, impl::need_syntax(impl::ESyntax::Ilike)>);
    STATIC_REQUIRE(impl::SupportsNeeds<backend, impl::need_function("now", 0)>);
    STATIC_REQUIRE(!impl::SupportsNeeds<backend, impl::need_function("greatest", 1)>);
}

FLUXMT_TEST_CASE("sqlite backend accepts typed clauses", "[sqlite]") {
    using namespace flux::orm;

    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::AcceptsClause<backend, &flux::kinds::select, decltype(where(col("a").ilike("x%")))>);
    STATIC_REQUIRE(impl::AcceptsClause<backend, &flux::kinds::insert, decltype(returning("id"))>);
    STATIC_REQUIRE(impl::AcceptsClause<backend, &flux::kinds::insert, decltype(do_update("a", "b"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<backend, &flux::kinds::insert, decltype(on_conflict_constraint("c"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<backend, &flux::kinds::select, decltype(returning("id"))>);
}
