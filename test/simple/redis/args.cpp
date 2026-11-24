// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <stdexcept>
#include <string>

// fluxmt
#include <fluxmt/redis.hpp>

// test
#include "support/test_case.hpp"


namespace {
    using flux::impl::redis::to_arg;
}


FLUXMT_TEST_CASE("redis scalar arguments", "[redis]") {
    REQUIRE(to_arg(flux::Value(std::string("hello"))) == "hello");
    REQUIRE(to_arg(true) == "1");
    REQUIRE(to_arg(false) == "0");
    REQUIRE(to_arg(ers::i64{ -7 }) == "-7");
    REQUIRE(to_arg(ers::u64{ 18446744073709551615u }) == "18446744073709551615");
    REQUIRE(to_arg(1.5) == "1.5");
}

FLUXMT_TEST_CASE("redis binary arguments keep every byte", "[redis]") {
    const flux::value_bytes_t blob{ std::byte{ 'a' }, std::byte{ 0 }, std::byte{ 'b' } };

    REQUIRE(to_arg(blob) == std::string("a\0b", 3));
}

FLUXMT_TEST_CASE("redis time and uuid arguments are text", "[redis]") {
    const auto day = std::chrono::sys_days{ std::chrono::year(2026) / 11 / 24 };
    const ers::timestamp_t moment = day + std::chrono::hours(12) + std::chrono::minutes(30);
    const ers::uuid id = ers::uuid_from_string("123e4567-e89b-12d3-a456-426614174000");

    REQUIRE(to_arg(ers::date_t{ day }) == "2026-11-24");
    REQUIRE(to_arg(moment) == "2026-11-24 12:30:00.000000");
    REQUIRE(to_arg(id) == ers::uuid_to_string(id));
}

FLUXMT_TEST_CASE("redis rejects arguments it cannot send", "[redis]") {
    REQUIRE_THROWS_AS(to_arg(flux::Value{}), std::invalid_argument);
    REQUIRE_THROWS_AS(to_arg(flux::value_array_t{ 1, 2 }), std::invalid_argument);
}
