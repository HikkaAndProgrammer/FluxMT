// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <vector>

// fluxmt
#include <fluxmt/drivers/psql.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;


namespace {
    using flux::sql::ESyntax;
    using flux::sql::needs_of;
    using flux::sql::supports;

    constexpr auto& pg = flux::impl::psql::postgres_syntax;

    template<typename E>
    inline constexpr flux::sql::needs_t needs = needs_of<E>();
}


static_assert(supports(pg, needs<decltype(col("a").ilike("x%"))>));
static_assert(supports(pg, needs<decltype(col("a").any(std::vector<int>{}))>));
static_assert(supports(pg, needs<decltype(col("a").cast("uuid") == col("b"))>));
static_assert(supports(pg, flux::impl::need_syntax(ESyntax::Excluded)));

// A function keeps its own name
static_assert(supports(pg, needs<decltype(now())>));
static_assert(supports(pg, needs<decltype(greatest(col("a")))>));
static_assert(supports(pg, needs<decltype(coalesce(col("a"), 0))>));


FLUXMT_TEST_CASE("psql dialect is built on its syntax table", "[psql]") {
    const auto& syntax = flux::psql::dialect().syntax();

    CHECK(syntax.cast == pg.cast);
    CHECK(syntax.ilike == pg.ilike);
    CHECK(syntax.any == pg.any);
    CHECK(syntax.excluded == pg.excluded);
    CHECK(syntax.function == nullptr);
}
