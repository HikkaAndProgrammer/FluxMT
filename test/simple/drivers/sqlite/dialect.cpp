// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>

// fluxmt
#include <fluxmt/sql.hpp>
#include <fluxmt/drivers/sqlite.hpp>

// test
#include "support/test_case.hpp"


using backend = flux::sqlite::backend;
using traits = flux::impl::backend_traits<backend>;

static_assert(flux::sql::SqlBackend<backend>);


FLUXMT_TEST_CASE("sqlite placeholders are numbered", "[sqlite]") {
    REQUIRE(flux::sqlite::dialect().placeholder(1) == "?1");
    REQUIRE(flux::sqlite::dialect().placeholder(12) == "?12");
}

FLUXMT_TEST_CASE("sqlite knows its reserved words", "[sqlite]") {
    const auto& dialect = flux::sqlite::dialect();

    for (const auto word : { "select", "order", "group", "table", "key", "limit", "returning" })
        REQUIRE(dialect.is_reserved(word));

    for (const auto word : { "users", "name", "id", "created_at", "user", "value" })
        REQUIRE(!dialect.is_reserved(word));

    REQUIRE(std::ranges::is_sorted(flux::sqlite::reserved_words()));
}

FLUXMT_TEST_CASE("sqlite quotes only what needs it", "[sqlite]") {
    flux::sql::Context ctx(flux::sqlite::dialect());

    REQUIRE(ctx.append_identifier("main.order"));
    ctx.query += ' ';
    REQUIRE(ctx.append_identifier("Account"));
    ctx.query += ' ';
    REQUIRE(ctx.append_identifier("created_at"));

    REQUIRE(ctx.query == "main.\"order\" \"Account\" created_at");
    REQUIRE(ctx.bind("x") == "?1");
}

FLUXMT_TEST_CASE("sqlite capabilities match its features", "[sqlite]") {
    const auto& caps = flux::sqlite::dialect().caps();

    REQUIRE(!caps.native_arrays);
    REQUIRE(!caps.ilike);
    REQUIRE(caps.cast == flux::sql::ECastStyle::Function);
    REQUIRE(traits::features.contains(flux::EFeature::NativeArrays) == caps.native_arrays);
}
