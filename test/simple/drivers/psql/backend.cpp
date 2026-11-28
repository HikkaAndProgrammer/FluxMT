// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <concepts>

// pqxx
#include <pqxx/connection>

// fluxmt
#include <fluxmt/drivers/psql.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("psql backend", "[psql]") {
    using B = flux::psql::backend;

    STATIC_REQUIRE(flux::IsBackend<B>);
    STATIC_REQUIRE(std::same_as<flux::connection_t<B>, pqxx::connection>);
    STATIC_REQUIRE(flux::ext::backend_traits<B>::name == "psql");

    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::Sql>);
    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::Transactions>);
    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::Returning>);
    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::Upsert>);
    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::DistinctOn>);
    STATIC_REQUIRE(flux::SupportsFeature<B, flux::EFeature::NativeArrays>);
}

FLUXMT_TEST_CASE("psql backend is a sql backend at compile time", "[psql]") {
    using B = flux::psql::backend;

    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::IsSqlBackend<B>);

    STATIC_REQUIRE(impl::HasSlot<B, impl::psql::insert_layout, &flux::slots::returning>);
    STATIC_REQUIRE(impl::HasSlot<B, impl::psql::insert_layout, &flux::slots::conflict_constraint>);
    STATIC_REQUIRE(!impl::HasSlot<B, impl::psql::select_layout, &flux::slots::returning>);

    STATIC_REQUIRE(impl::SupportsNeeds<B, impl::need_syntax(impl::ESyntax::Ilike)>);
    STATIC_REQUIRE(impl::SupportsNeeds<B, impl::need_syntax(impl::ESyntax::Any)>);
}

FLUXMT_TEST_CASE("psql backend accepts typed clauses", "[psql]") {
    using B = flux::psql::backend;
    using namespace flux::orm;

    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::AcceptsClause<B, impl::psql::select_layout, decltype(where(col("a").ilike("x%")))>);
    STATIC_REQUIRE(impl::AcceptsClause<B, impl::psql::insert_layout, decltype(on_conflict_constraint("c"))>);
    STATIC_REQUIRE(impl::AcceptsClause<B, impl::psql::insert_layout, decltype(do_update("a", "b"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<B, impl::psql::select_layout, decltype(returning("id"))>);
}

FLUXMT_TEST_CASE("psql backend accepts DISTINCT ON in a select only", "[psql]") {
    using B = flux::psql::backend;

    namespace impl = flux::impl;

    STATIC_REQUIRE(impl::AcceptsClause<B, impl::psql::select_layout, decltype(impl::psql::distinct_on("a"))>);
    STATIC_REQUIRE(!impl::AcceptsClause<B, impl::psql::insert_layout, decltype(impl::psql::distinct_on("a"))>);
}
