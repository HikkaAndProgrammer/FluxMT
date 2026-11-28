// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/sql.hpp>
#include <fluxmt/drivers/sqlite.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;


namespace {
    std::string sql_of(const flux::sql::Expr& expr) {
        flux::ext::SqlContext ctx(flux::sqlite::dialect());

        return expr.render(ctx) ? ctx.text : "<error>";
    }
}


FLUXMT_TEST_CASE("sqlite expr placeholders", "[sqlite]") {
    CHECK(sql_of(col("a") > 1 && col("b") == "x") == "a > ?1 AND b = ?2");
}

FLUXMT_TEST_CASE("sqlite expr falls back where it lacks a feature", "[sqlite]") {
    CHECK(sql_of(col("a").ilike("x%")) == "lower(a) LIKE lower(?1)");
    CHECK(sql_of(col("a").any(std::vector<int>{ 1, 2 })) == "a IN (?1, ?2)");
    CHECK(sql_of(col("a").any(std::vector<int>{})) == "FALSE");
    CHECK(sql_of(col("a").cast("text") == col("b")) == "CAST(a AS text) = b");
}

FLUXMT_TEST_CASE("sqlite expr spells functions its own way", "[sqlite]") {
    CHECK(sql_of(now()) == "CURRENT_TIMESTAMP");
    CHECK(sql_of(col("a") < now()) == "a < CURRENT_TIMESTAMP");
    CHECK(sql_of(greatest(col("a"), col("b"))) == "max(a, b)");
    CHECK(sql_of(least(col("a"), col("b"), 3)) == "min(a, b, ?1)");
    CHECK(sql_of(coalesce(col("a"), 0)) == "coalesce(a, ?1)");
}

FLUXMT_TEST_CASE("sqlite expr refuses greatest and least of one argument", "[sqlite]") {
    flux::ext::SqlContext ctx(flux::sqlite::dialect());

    const auto r = greatest(col("a")).render(ctx);

    REQUIRE(!r);
    CHECK(r.error().code() == flux::EQueryError::Unsupported);
}

FLUXMT_TEST_CASE("sqlite expr spreads an array into parameters", "[sqlite]") {
    flux::ext::SqlContext ctx(flux::sqlite::dialect());

    REQUIRE((col("a").any(std::vector<int>{ 1, 2 })).render(ctx));
    REQUIRE(ctx.params() == std::vector<flux::Value>{ 1, 2 });
}

FLUXMT_TEST_CASE("sqlite expr quotes reserved words", "[sqlite]") {
    CHECK(sql_of(col("order") == 1) == "\"order\" = ?1");
    CHECK(sql_of(col("t", "key") == 1) == "t.\"key\" = ?1");
    CHECK(sql_of(col("user") == 1) == "user = ?1");
    CHECK(sql_of(col("created_at") == 1) == "created_at = ?1");
}
