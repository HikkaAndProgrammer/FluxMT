// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/psql.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;

using std::string_literals::operator ""s;


namespace {
    ers::Result<flux::sql::Statement> statement_of(const flux::sql::Query& query) {
        return query.to_statement(flux::psql::dialect());
    }

    std::string sql_of(const flux::sql::Query& query) {
        const auto r = statement_of(query);

        return r ? (*r).sql : std::string("<error>");
    }
}


FLUXMT_TEST_CASE("psql query placeholders", "[psql]") {
    CHECK(sql_of(select_from("users") | where(col("active") == true) | with_limit(5))
        == "SELECT *\nFROM users\nWHERE active = $1\nLIMIT $2");
}

FLUXMT_TEST_CASE("psql query upsert", "[psql]") {
    CHECK(sql_of(insert_into("images")
        | columns("url", "path")
        | values("a.png"s, "/a.png"s)
        | on_conflict("url")
        | do_update("path")
        | returning("id"))
        == "INSERT INTO images (url, path)"
           "\nVALUES ($1, $2)"
           "\nON CONFLICT (url)"
           "\nDO UPDATE SET path = excluded.path"
           "\nRETURNING id");
}

FLUXMT_TEST_CASE("psql query quotes reserved words", "[psql]") {
    CHECK(sql_of(select_from("user") | where(col("order") == 1))
        == "SELECT *\nFROM \"user\"\nWHERE \"order\" = $1");

    CHECK(sql_of(select_from("t") | column(col("a").as("limit"))) == "SELECT a AS \"limit\"\nFROM t");
}

FLUXMT_TEST_CASE("psql query binds an array as one parameter", "[psql]") {
    const auto r = statement_of(select_from("t") | where(col("id").any(std::vector<int>{ 1, 2 })));

    REQUIRE(r);
    CHECK((*r).sql == "SELECT *\nFROM t\nWHERE id = ANY($1)");
    CHECK((*r).params == std::vector<flux::Value>{ flux::Value(flux::value_array_t{ 1, 2 }) });
}

FLUXMT_TEST_CASE("psql query subquery numbers parameters through the whole statement", "[psql]") {
    CHECK(sql_of(select_from("users")
        | where(col("active") == true)
        | where(col("id").in(select_from("orders") | columns("user_id") | where(col("total") > 100)))
        | with_limit(3))
        == "SELECT *\nFROM users"
           "\nWHERE active = $1 AND id IN (SELECT user_id\nFROM orders\nWHERE total > $2)"
           "\nLIMIT $3");
}
