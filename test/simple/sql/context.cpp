// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


FLUXMT_TEST_CASE("sql context numbers parameters from one", "[sql]") {
    test::AntiDialect dialect;
    flux::sql::Context ctx(dialect);

    REQUIRE(ctx.bind(1) == "?1");
    REQUIRE(ctx.bind(std::string("a")) == "?2");

    const auto statement = std::move(ctx).finish();

    REQUIRE(statement.params == std::vector<flux::Value>{ 1, std::string("a") });
}

FLUXMT_TEST_CASE("sql context reuses a placeholder by key", "[sql]") {
    test::AntiDialect dialect;
    flux::sql::Context ctx(dialect);

    int first = 0;
    int second = 0;

    REQUIRE(ctx.bind_once(&first, 10) == "?1");
    REQUIRE(ctx.bind_once(&second, 20) == "?2");
    REQUIRE(ctx.bind_once(&first, 99) == "?1");

    REQUIRE(ctx.params() == std::vector<flux::Value>{ 10, 20 });
}

FLUXMT_TEST_CASE("sql context finish moves the text and parameters out", "[sql]") {
    test::AntiDialect dialect;
    flux::sql::Context ctx(dialect);

    ctx.query = "SELECT " + ctx.bind(5);

    const auto statement = std::move(ctx).finish();

    REQUIRE(statement == flux::sql::Statement{ "SELECT ?1",{ 5 } });
}

FLUXMT_TEST_CASE("sql context writes identifiers by the dialect policy", "[sql]") {
    test::AntiDialect dialect;

    const auto render = [&](std::string_view name) {
        flux::sql::Context ctx(dialect);
        ctx.append_identifier(name);

        return ctx.query;
        };

    REQUIRE(render("users") == "users");
    REQUIRE(render("order") == "order");
    REQUIRE(render("item") == "\"item\"");
    REQUIRE(render("Users") == "\"Users\"");
    REQUIRE(render("a b") == "\"a b\"");
    REQUIRE(render("public.users") == "public.users");
    REQUIRE(render("public.group") == "public.\"group\"");
    REQUIRE(render("Public.Users") == "\"Public\".\"Users\"");
}

FLUXMT_TEST_CASE("sql context rejects empty identifier parts", "[sql]") {
    test::AntiDialect dialect;
    flux::sql::Context ctx(dialect);

    REQUIRE_THROWS_AS(ctx.append_identifier(""), std::invalid_argument);
    REQUIRE_THROWS_AS(ctx.append_identifier("a..b"), std::invalid_argument);
    REQUIRE_THROWS_AS(ctx.append_identifier(".a"), std::invalid_argument);
    REQUIRE_THROWS_AS(ctx.append_identifier("a."), std::invalid_argument);

    REQUIRE(ctx.query.empty());
}
