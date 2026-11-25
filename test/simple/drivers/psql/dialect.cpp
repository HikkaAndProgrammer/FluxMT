// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>

// fluxmt
#include <fluxmt/psql.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


static_assert(flux::sql::SqlBackend<flux::psql::backend>);


FLUXMT_TEST_CASE("psql placeholders start from one", "[psql]") {
    REQUIRE(flux::psql::dialect().placeholder(1) == "$1");
    REQUIRE(flux::psql::dialect().placeholder(12) == "$12");
}

FLUXMT_TEST_CASE("psql knows its reserved words", "[psql]") {
    const auto& dialect = flux::psql::dialect();

    for (const auto word : { "select", "order", "user", "group", "table", "inner", "ilike", "with" })
        REQUIRE(dialect.is_reserved(word));

    for (const auto word : { "users", "name", "value", "key", "id", "created_at" })
        REQUIRE(!dialect.is_reserved(word));

    REQUIRE(std::ranges::is_sorted(flux::psql::reserved_words()));
}

FLUXMT_TEST_CASE("psql quotes only what needs it", "[psql]") {
    flux::sql::Context ctx(flux::psql::dialect());

    REQUIRE(ctx.append_identifier("public.order"));
    ctx.query += ' ';
    REQUIRE(ctx.append_identifier("Account"));
    ctx.query += ' ';
    REQUIRE(ctx.append_identifier("created_at"));

    REQUIRE(ctx.query == "public.\"order\" \"Account\" created_at");
    REQUIRE(ctx.bind("x") == "$1");
}
