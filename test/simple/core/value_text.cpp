// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/core/value_text.hpp>

// test
#include "support/test_case.hpp"


FLUXMT_TEST_CASE("value text forms", "[core]") {
    const auto day = std::chrono::sys_days{ std::chrono::year(2026) / 11 / 24 };
    const ers::timestamp_t moment = day + std::chrono::hours(12) + std::chrono::minutes(30);
    const ers::uuid id = ers::uuid_from_string("123e4567-e89b-12d3-a456-426614174000");

    REQUIRE(flux::to_text(ers::date_t{ day }) == "2026-11-24");
    REQUIRE(flux::to_text(moment) == "2026-11-24 12:30:00.000000");
    REQUIRE(flux::to_text(id) == ers::uuid_to_string(id));
}
