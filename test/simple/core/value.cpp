// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/test_case.hpp"


FLUXMT_TEST_CASE("value alternatives", "[core]") {
    REQUIRE(flux::Value{}.is_null());
    REQUIRE(*flux::Value{ true }.get_if<bool>());
    REQUIRE(*flux::Value{ ers::i64{ 7 } }.get_if<ers::i64>() == 7);
    REQUIRE(*flux::Value{ std::string("x") }.get_if<std::string>() == "x");
    REQUIRE(flux::Value{ 7 }.get_if<ers::i64>() != nullptr);
    REQUIRE(flux::Value{ "text" }.get_if<std::string>() != nullptr);
}

FLUXMT_TEST_CASE("value equality", "[core]") {
    REQUIRE(flux::Value{ 1 } == flux::Value{ 1 });
    REQUIRE(flux::Value{ 1 } != flux::Value{ 2 });
    REQUIRE(flux::Value{} == flux::Value{});
    REQUIRE(flux::Value{ flux::value_array_t{ 1, 2 } } == flux::Value{ flux::value_array_t{ 1, 2 } });
}
