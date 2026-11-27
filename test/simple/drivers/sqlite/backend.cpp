// catch2
#include <catch2/catch_test_macros.hpp>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using backend = flux::sqlite::backend;

static_assert(flux::IsBackend<backend>);
static_assert(flux::IsConnectable<backend>);
static_assert(flux::SupportsFeature<backend, flux::EFeature::Returning>);
static_assert(flux::SupportsFeature<backend, flux::EFeature::Upsert>);
static_assert(!flux::SupportsFeature<backend, flux::EFeature::DistinctOn>);
static_assert(!flux::SupportsFeature<backend, flux::EFeature::NativeArrays>);


FLUXMT_TEST_CASE("sqlite backend traits", "[sqlite]") {
    using traits = flux::impl::backend_traits<backend>;

    REQUIRE(traits::name == "sqlite");
    REQUIRE(traits::features.contains(flux::EFeature::Sql));
    REQUIRE(traits::features.contains(flux::EFeature::Transactions));
    REQUIRE(!traits::features.contains(flux::EFeature::DistinctOn));
}

FLUXMT_TEST_CASE("sqlite backend is a sql backend at compile time", "[sqlite]") {
    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::IsSqlBackend<backend>);

    STATIC_REQUIRE(impl::HasSlot<backend, impl::sqlite::insert_layout, &flux::slots::returning>);
    STATIC_REQUIRE(!impl::HasSlot<backend, impl::sqlite::insert_layout, &flux::slots::conflict_constraint>);
    STATIC_REQUIRE(!impl::HasSlot<backend, impl::sqlite::select_layout, &flux::slots::returning>);

    STATIC_REQUIRE(impl::SupportsNeeds<backend, impl::need_syntax(impl::ESyntax::Ilike)>);
    STATIC_REQUIRE(impl::SupportsNeeds<backend, impl::need_function("now", 0)>);
    STATIC_REQUIRE(!impl::SupportsNeeds<backend, impl::need_function("greatest", 1)>);
}

FLUXMT_TEST_CASE("sqlite backend accepts typed clauses", "[sqlite]") {
    using namespace flux::orm;

    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::AcceptsClause<backend, impl::sqlite::select_layout, decltype(where(col("a").ilike("x%")))>);
    STATIC_REQUIRE(impl::AcceptsClause<backend, impl::sqlite::insert_layout, decltype(returning("id"))>);
    STATIC_REQUIRE(impl::AcceptsClause<backend, impl::sqlite::insert_layout, decltype(do_update("a", "b"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<backend, impl::sqlite::insert_layout, decltype(on_conflict_constraint("c"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<backend, impl::sqlite::select_layout, decltype(returning("id"))>);
}
