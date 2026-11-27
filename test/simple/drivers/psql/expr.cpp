// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/drivers/psql.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;


namespace {
    std::string sql_of(const flux::sql::Expr& expr) {
        flux::ext::SqlContext ctx(flux::psql::dialect());

        return expr.render(ctx) ? ctx.text : "<error>";
    }
}


FLUXMT_TEST_CASE("psql expr placeholders", "[psql]") {
    CHECK(sql_of(col("a") > 1 && col("b") == "x") == "a > $1 AND b = $2");
}

FLUXMT_TEST_CASE("psql expr uses its capabilities", "[psql]") {
    CHECK(sql_of(col("a").ilike("x%")) == "a ILIKE $1");
    CHECK(sql_of(col("a").any(std::vector<int>{ 1, 2 })) == "a = ANY($1)");
    CHECK(sql_of(col("a").cast("uuid") == col("b")) == "a::uuid = b");
    CHECK(sql_of((col("a") + col("b")).cast("int") > 1) == "(a + b)::int > $1");
}

FLUXMT_TEST_CASE("psql expr keeps function names", "[psql]") {
    CHECK(sql_of(now()) == "now()");
    CHECK(sql_of(greatest(col("a"), col("b"))) == "greatest(a, b)");
    CHECK(sql_of(least(col("a"), 3)) == "least(a, $1)");
}

FLUXMT_TEST_CASE("psql expr binds an array as one parameter", "[psql]") {
    flux::ext::SqlContext ctx(flux::psql::dialect());

    REQUIRE((col("a").any(std::vector<int>{ 1, 2 })).render(ctx));
    REQUIRE(ctx.params().size() == 1);
    REQUIRE(ctx.params()[0] == flux::Value(flux::value_array_t{ 1, 2 }));
}

FLUXMT_TEST_CASE("psql expr quotes reserved words", "[psql]") {
    CHECK(sql_of(col("user") == 1) == "\"user\" = $1");
    CHECK(sql_of(col("t", "order") == 1) == "t.\"order\" = $1");
    CHECK(sql_of(col("created_at") == 1) == "created_at = $1");
}
