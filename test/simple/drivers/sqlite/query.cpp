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

using std::string_literals::operator ""s;


namespace {
    ers::Result<flux::sql::statement_t> statement_of(const flux::sql::Query& query) {
        return query.to_statement(flux::sqlite::dialect());
    }

    std::string sql_of(const flux::sql::Query& query) {
        const auto r = statement_of(query);

        return r ? (*r).text : std::string("<error>");
    }
}


FLUXMT_TEST_CASE("sqlite query placeholders", "[sqlite]") {
    CHECK(sql_of(select_from("users") | where(col("active") == true) | with_limit(5))
        == "SELECT *\nFROM users\nWHERE active = ?1\nLIMIT ?2");
}

FLUXMT_TEST_CASE("sqlite query upsert", "[sqlite]") {
    CHECK(sql_of(insert_into("images")
        | columns("url", "path")
        | values("a.png"s, "/a.png"s)
        | on_conflict("url")
        | do_update("path")
        | returning("id"))
        == "INSERT INTO images (url, path)"
        "\nVALUES (?1, ?2)"
        "\nON CONFLICT (url)"
        "\nDO UPDATE SET path = excluded.path"
        "\nRETURNING id");
}

FLUXMT_TEST_CASE("sqlite query quotes reserved words", "[sqlite]") {
    CHECK(sql_of(select_from("user") | where(col("order") == 1))
        == "SELECT *\nFROM user\nWHERE \"order\" = ?1");

    CHECK(sql_of(select_from("t") | column(col("a").as("limit"))) == "SELECT a AS \"limit\"\nFROM t");
}

FLUXMT_TEST_CASE("sqlite query writes OFFSET after a LIMIT", "[sqlite]") {
    CHECK(sql_of(select_from("t") | with_offset(5)) == "SELECT *\nFROM t\nLIMIT -1\nOFFSET ?1");
    CHECK(sql_of(select_from("t") | with_limit(2) | with_offset(5)) == "SELECT *\nFROM t\nLIMIT ?1\nOFFSET ?2");
    CHECK(sql_of(select_from("t") | with_limit(2)) == "SELECT *\nFROM t\nLIMIT ?1");

    CHECK(sql_of(select_from("u") | where(col("id").in(select_from("o") | columns("user_id") | with_offset(1))))
        == "SELECT *\nFROM u"
           "\nWHERE id IN (SELECT user_id\nFROM o\nLIMIT -1\nOFFSET ?1)");
}

FLUXMT_TEST_CASE("sqlite query refuses ON CONSTRAINT", "[sqlite]") {
    const auto refused = statement_of(insert_into("t")
        | columns("a")
        | values(1)
        | on_conflict_constraint("t_a_key")
        | do_nothing());

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::Unsupported);

    CHECK(sql_of(insert_into("t") | columns("a") | values(1) | on_conflict("a") | do_nothing())
        == "INSERT INTO t (a)\nVALUES (?1)\nON CONFLICT (a)\nDO NOTHING");
}

FLUXMT_TEST_CASE("sqlite query spreads an array into parameters", "[sqlite]") {
    const auto r = statement_of(select_from("t") | where(col("id").any(std::vector<int>{ 1, 2 })));

    REQUIRE(r);
    CHECK((*r).text == "SELECT *\nFROM t\nWHERE id IN (?1, ?2)");
    CHECK((*r).params == std::vector<flux::Value>{ 1, 2 });
}
