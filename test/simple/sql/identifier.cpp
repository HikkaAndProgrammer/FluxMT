// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string_view>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


namespace {
    using flux::sql::is_plain_identifier;
    using flux::sql::quote_identifier;
}


FLUXMT_TEST_CASE("sql plain identifiers", "[sql]") {
    REQUIRE(is_plain_identifier("users"));
    REQUIRE(is_plain_identifier("_tmp1"));
    REQUIRE(is_plain_identifier("created_at"));

    REQUIRE(!is_plain_identifier(""));
    REQUIRE(!is_plain_identifier("Users"));
    REQUIRE(!is_plain_identifier("1abc"));
    REQUIRE(!is_plain_identifier("a-b"));
    REQUIRE(!is_plain_identifier("a b"));
    REQUIRE(!is_plain_identifier("a.b"));
    REQUIRE(!is_plain_identifier("имя"));
}

FLUXMT_TEST_CASE("sql quotes identifiers with doubled quotes", "[sql]") {
    REQUIRE(quote_identifier("order") == "\"order\"");
    REQUIRE(quote_identifier("Mixed Case") == "\"Mixed Case\"");
    REQUIRE(quote_identifier("a\"b") == "\"a\"\"b\"");
}

FLUXMT_TEST_CASE("sql refuses identifiers it cannot quote", "[sql]") {
    REQUIRE_THROWS_AS(quote_identifier(""), std::invalid_argument);
    REQUIRE_THROWS_AS(quote_identifier(std::string_view("a\0b", 3)), std::invalid_argument);
}

FLUXMT_TEST_CASE("sql function and type names", "[sql]") {
    using flux::sql::is_type_name;
    using flux::sql::is_valid_name;

    REQUIRE(is_valid_name("lower"));
    REQUIRE(is_valid_name("Coalesce"));
    REQUIRE(is_valid_name("pg_catalog.now"));
    REQUIRE(!is_valid_name(""));
    REQUIRE(!is_valid_name("1a"));
    REQUIRE(!is_valid_name("a b"));
    REQUIRE(!is_valid_name("a.."));
    REQUIRE(!is_valid_name("a; --"));

    REQUIRE(is_type_name("BIGINT"));
    REQUIRE(is_type_name("text[]"));
    REQUIRE(is_type_name("int[][]"));
    REQUIRE(!is_type_name("int; --"));
    REQUIRE(!is_type_name("[]"));
}
