// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>

// fluxmt
#include <fluxmt/drivers/psql.hpp>
#include <fluxmt/drivers/psql/params.hpp>

// test
#include "support/test_case.hpp"


using flux::Value;
using flux::impl::psql::to_text;


FLUXMT_TEST_CASE("scalar text", "[psql]") {
    REQUIRE(to_text(Value{}) == "NULL");
    REQUIRE(to_text(Value{ true }) == "true");
    REQUIRE(to_text(Value{ 42 }) == "42");
    REQUIRE(to_text(Value{ "a" }) == "a");
}

FLUXMT_TEST_CASE("array text", "[psql]") {
    REQUIRE(to_text(Value{ flux::value_array_t{ 1, Value{}, 3 } }) == R"({"1",NULL,"3"})");
    REQUIRE(to_text(Value{ flux::value_array_t{ "a\"b" } }) == R"({"a\"b"})");
    REQUIRE(to_text(Value{ flux::value_array_t{ flux::value_array_t{ 1, 2 }, flux::value_array_t{ 3 } } })
        == R"({{"1","2"},{"3"}})");
}

FLUXMT_TEST_CASE("params count", "[psql]") {
    const std::array values { Value { 1 }, Value {}, Value { "x" } };
    REQUIRE(flux::impl::psql::make_params(values).size() == 3);
}
